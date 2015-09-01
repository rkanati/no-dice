#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <stdexcept>

#include <EGL/egl.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <X11/Xlib-xcb.h>

using u32 = uint32_t;

template <typename value_t, typename func_t>
class guard_t {
  value_t& ref;
  func_t func;
  bool active;
public:
  guard_t (value_t& ref, func_t func) :
    ref (ref),
    func (func),
    active (true)
  { }
  guard_t (const guard_t&) = delete;
  guard_t (guard_t&& other) :
    ref (other.ref),
    func (other.func),
    active (other.active)
  {
    other.active = false;
  }
  void relieve () {
    active = false;
  }
  ~guard_t () {
    if (active)
      func (ref);
  }
};

template <typename value_t, typename func_t>
auto guard (value_t& ref, func_t func) {
  return guard_t <value_t, func_t> (ref, func);
}

class x11_objects_t {
public:
  Display* xlib_display;
  xcb_connection_t* xcb_connection;
  xcb_window_t window;
  xcb_atom_t wm_delete_atom;

  ~x11_objects_t () {
    XCloseDisplay (xlib_display);
  }
};

x11_objects_t setup_x (int win_width, int win_height) {
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
    XCB_EVENT_MASK_EXPOSURE
  };

  xcb_window_t win = xcb_generate_id (conn);
  xcb_void_cookie_t create_win_cookie = xcb_create_window_checked (
    conn,
    XCB_COPY_FROM_PARENT,
    win,
    screen->root,
    0, 0, win_width, win_height,
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

  disp_guard.relieve ();

  return x11_objects_t { disp, conn, win, wm_delete_atom };
}

int main () {
  x11_objects_t x11 = setup_x (800, 600);
  xcb_flush (x11.xcb_connection);

  xcb_generic_event_t* ev;
  while ((ev = xcb_wait_for_event (x11.xcb_connection))) {
    switch (XCB_EVENT_RESPONSE_TYPE (ev)) {
      case XCB_EXPOSE:
        printf ("oh my\n");
        break;
      case XCB_CLIENT_MESSAGE: {
        auto cmev = (xcb_client_message_event_t*) ev;
        auto msg = cmev->data.data32[0];
        if (msg == x11.wm_delete_atom) {
          free (ev);
          goto exit_loop;
        }
      }
      default:;
    }
    free (ev);
  }

exit_loop:;
}

