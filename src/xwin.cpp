//
// no-dice
//

#include "xwin.hpp"

#include "guard.hpp"
#include "types.hpp"

#include <stdexcept>
#include <cstdlib>
#include <cstring>

#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_keysyms.h>
#include <X11/Xlib-xcb.h>

namespace nd {
  class Keyboard final : public InputDevice {
    uint            pending_release_key;
    xcb_timestamp_t pending_release_time;

  public:
    Keyboard () :
      pending_release_key (0)
    { }

    void handle_key (xcb_key_press_event_t const* ev, InputEventQueue& out) {
      bool down = (ev->response_type == XCB_KEY_PRESS);
      // auto sym = xcb_key_symbols_get_keysym (key_syms, kev->detail, 0);

      // ugly hack to get around Xorg's refusal to tell you anything physical about keys
      // in a consistent way. assumes evdev/hid/xkb-ish codes with the xf86-input-evdev
      // style offset.
      auto code = (uint) ev->detail - 8;

      if (down) {
        // release-then-press with the same timestamp is a key repeat. ignore it.
        if (ev->time == pending_release_time && code == pending_release_key) {
          pending_release_key = 0;
        }
        else {
          // flush the pending release event
          flush (out);
          out.push_back (InputEvent { { this, code }, InputType::bistate, false, down });
        }
      }
      else {
        // defer release events for repeat detection
        flush (out);
        pending_release_key = code;
        pending_release_time = ev->time;
      }
    }

    void flush (InputEventQueue& out) {
      if (!pending_release_key)
        return;

      out.push_back (InputEvent { { this, pending_release_key }, InputType::bistate, false, false });
      pending_release_key = 0;
    }
  };

  class Pointer final : public InputDevice {
  public:
    auto handle_button (xcb_button_press_event_t const* ev) {
      bool down = (ev->response_type == XCB_BUTTON_PRESS);
      return InputEvent { { this, ev->detail }, InputType::bistate, false, down };
    }

    auto handle_motion (xcb_motion_notify_event_t const* ev) {
      auto coords = v2i { ev->event_x, ev->event_y };
      return InputEvent { { this, 0 }, InputType::axial_2i, false, { .value_2i = coords } };
    }
  };

  class XHostImpl final : public XHost {
    Display*           const disp;
    xcb_connection_t*  const conn;
    xcb_window_t       const win;
    xcb_atom_t         const wm_delete_atom;
    xcb_key_symbols_t* const key_syms;

    Keyboard keyb;
    Pointer point;

    int wide, high;

  public:
    XHostImpl (Display* d, xcb_connection_t* c, xcb_window_t w, xcb_atom_t wmda, xcb_key_symbols_t* ks) :
      disp (d),
      conn (c),
      win (w),
      wm_delete_atom (wmda),
      key_syms (ks),
      wide (1024),
      high (768)
    { }

    ~XHostImpl () {
      XCloseDisplay (disp);
    }

    void handle_event (InputFrame& inframe, xcb_generic_event_t const* ev) {
      auto resp_type = XCB_EVENT_RESPONSE_TYPE (ev);

      switch (resp_type) {
        case XCB_CLIENT_MESSAGE: {
          auto cmev = (xcb_client_message_event_t*) ev;
          auto msg = cmev->data.data32[0];
          if (msg == wm_delete_atom)
            inframe.quit = true;
        } return;

        case XCB_MOTION_NOTIFY: {
          auto out = point.handle_motion ((xcb_motion_notify_event_t const*) ev);
          inframe.events.push_back (out);
        } return;

        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE: {
          auto out = point.handle_button ((xcb_button_press_event_t const*) ev);
          inframe.events.push_back (out);
        } return;

        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE: {
          keyb.handle_key ((xcb_key_press_event_t const*) ev, inframe.events);
        } return;

        default:;
      }
    }

    InputFrame pump () override {
      xcb_generic_error_t* xcb_err;
      InputFrame inframe;

      xcb_generic_event_t* ev;
      while ((ev = xcb_poll_for_event (conn))) {
        handle_event (inframe, ev);
        free (ev);
      }

      keyb.flush (inframe.events);

      auto get_geom_cookie = xcb_get_geometry (conn, win);
      auto* geom = xcb_get_geometry_reply (conn, get_geom_cookie, &xcb_err);
      if (geom) {
        wide = geom->width;
        high = geom->height;
        free (geom);
      }

      return inframe;
    }

    v2i dims () const override {
      return { wide, high };
    }

    EGLNativeDisplayType egl_display () override {
      return disp;
    }

    EGLNativeWindowType egl_window () override {
      return win;
    }

    InputDevice* keyboard () override {
      return &keyb;
    }

    InputDevice* pointer () override {
      return &point;
    }
  };

  XHost::Ptr XHost::create () {
    Display* disp = XOpenDisplay (nullptr);
    if (!disp)
      throw std::runtime_error ("XOpenDisplay failed");

    auto disp_guard = guard (disp, XCloseDisplay);

    xcb_connection_t* conn = XGetXCBConnection (disp);
    if (!conn)
      throw std::runtime_error ("XGetXCBConnection failed");
    if (xcb_connection_has_error (conn))
      throw std::runtime_error ("xcb_connection has errors");

    xcb_screen_t* screen = xcb_setup_roots_iterator (xcb_get_setup (conn)).data;

    const u32 attr_mask = XCB_CW_EVENT_MASK;
    const u32 attr_list[] = {
      XCB_EVENT_MASK_BUTTON_PRESS
      | XCB_EVENT_MASK_BUTTON_RELEASE
      | XCB_EVENT_MASK_POINTER_MOTION
      | XCB_EVENT_MASK_KEY_PRESS
      | XCB_EVENT_MASK_KEY_RELEASE
    };

    xcb_window_t win = xcb_generate_id (conn);
    xcb_void_cookie_t create_win_cookie = xcb_create_window_checked (
      conn,
      XCB_COPY_FROM_PARENT,
      win,
      screen->root,
      0, 0, 1024, 768,
      0,
      XCB_WINDOW_CLASS_INPUT_OUTPUT,
      screen->root_visual,
      attr_mask,
      attr_list);

    xcb_generic_error_t* err = xcb_request_check (conn, create_win_cookie);
    if (err)
      throw std::runtime_error ("xcb_create_window_checked failed");

    const char* wm_delete = "WM_DELETE_WINDOW";
    xcb_intern_atom_cookie_t wm_delete_intern_cookie =
      xcb_intern_atom (conn, 0, strlen (wm_delete), wm_delete);

    const char* wm_protocols = "WM_PROTOCOLS";
    xcb_intern_atom_cookie_t wm_proto_intern_cookie =
      xcb_intern_atom (conn, 1, strlen (wm_protocols), wm_protocols);

    xcb_atom_t wm_delete_atom =
      xcb_intern_atom_reply (conn, wm_delete_intern_cookie, 0)->atom;
    xcb_atom_t wm_protocols_atom =
      xcb_intern_atom_reply (conn, wm_proto_intern_cookie, 0)->atom;

    xcb_change_property (
      conn,
      XCB_PROP_MODE_REPLACE, win, wm_protocols_atom,
      XCB_ATOM_ATOM, 32, 1, &wm_delete_atom);

    xcb_void_cookie_t map_win_cookie = xcb_map_window_checked (conn, win);

    err = xcb_request_check (conn, map_win_cookie);
    if (err)
      throw std::runtime_error ("xcb_map_window_checked failed");

    xcb_key_symbols_t* keysyms = xcb_key_symbols_alloc (conn);
    if (!keysyms)
      throw std::runtime_error ("xcb_key_symbols_alloc failed");

    xcb_flush (conn);

    disp_guard.relieve ();
    return std::make_unique<XHostImpl> (disp, conn, win, wm_delete_atom, keysyms);
  }
}

