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
#include <X11/Xlib-xcb.h>

namespace nd {
  class XHost::Impl {
    Display* disp;
    xcb_connection_t* conn;
    xcb_window_t win;
    xcb_atom_t wm_delete_atom;

    int wide, high;

    Impl (Display* d, xcb_connection_t* c, xcb_window_t w, xcb_atom_t wmda) :
      disp (d), conn (c), win (w), wm_delete_atom (wmda),
      wide (1024), high (768)
    { }

  public:
    static Impl create ();
    ~Impl ();

    InputFrame pump ();

    int width () const {
      return wide;
    }

    int height () const {
      return high;
    }

    EGLNativeDisplayType egl_display () {
      return disp;
    }

    EGLNativeWindowType egl_window () {
      return win;
    }
  };

  XHost::XHost (Impl* i) :
    impl (i)
  { }

  XHost::Impl::~Impl () {
    XCloseDisplay (disp);
  }

  XHost::~XHost () {
    delete impl;
  }

  InputFrame XHost::Impl::pump () {
    xcb_generic_error_t* xcb_err;
    InputFrame inframe;

    xcb_generic_event_t* ev;
    while ((ev = xcb_poll_for_event (conn))) {
      switch (XCB_EVENT_RESPONSE_TYPE (ev)) {
        case XCB_CLIENT_MESSAGE: {
          auto cmev = (xcb_client_message_event_t*) ev;
          auto msg = cmev->data.data32[0];
          if (msg == wm_delete_atom) {
            inframe.quit = true;
          }
        }
        default:;
      }
      free (ev);
    }

    auto get_geom_cookie = xcb_get_geometry (conn, win);
    auto* geom = xcb_get_geometry_reply (conn, get_geom_cookie, &xcb_err);
    if (geom) {
      wide = geom->width;
      high = geom->height;
      free (geom);
    }

    return inframe;
  }

  InputFrame XHost::pump () {
    return impl->pump ();
  }

  int XHost::width () const {
    return impl->width ();
  }

  int XHost::height () const {
    return impl->height ();
  }

  EGLNativeDisplayType XHost::egl_display () {
    return impl->egl_display ();
  }

  EGLNativeWindowType XHost::egl_window () {
    return impl->egl_window ();
  }

  XHost::Impl XHost::Impl::create () {
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
      | XCB_EVENT_MASK_BUTTON_MOTION
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

    xcb_flush (conn);

    disp_guard.relieve ();
    return Impl (disp, conn, win, wm_delete_atom);
  }

  XHost XHost::create () {
    return new Impl (Impl::create ());
  }
}

