#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <stdexcept>

#include <EGL/egl.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <X11/Xlib-xcb.h>

#include <GL/gl.h>

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

class gl_objects_t {
public:
  EGLDisplay egl_display;
  EGLContext egl_context;
  EGLSurface egl_surface;
};

gl_objects_t setup_gl (x11_objects_t& x11) {
  auto ok = eglBindAPI (EGL_OPENGL_API);
  if (!ok)
    throw std::runtime_error ("eglBindAPI failed");

  auto egl_disp = eglGetDisplay (x11.xlib_display);
  if (egl_disp == EGL_NO_DISPLAY)
    throw std::runtime_error ("eglGetDisplay failed");

  int dummy;
  ok = eglInitialize (egl_disp, &dummy, &dummy);
  if (!ok)
    throw std::runtime_error ("eglInitialize failed");

  static const int config_attrs[] = {
    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
    EGL_BUFFER_SIZE,       32,
    EGL_DEPTH_SIZE,        24,
    EGL_SURFACE_TYPE,      EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE,   EGL_OPENGL_BIT,
    EGL_NONE
  };

  EGLConfig config;
  int config_count = 0;
  ok = eglChooseConfig (egl_disp, config_attrs, &config, 1, &config_count);
  if (!ok || config_count == 0)
    throw std::runtime_error ("eglChooseConfig failed");

  static const int context_attrs[] = {
    EGL_NONE
  };

  auto egl_ctx = eglCreateContext (egl_disp, config, EGL_NO_CONTEXT, context_attrs);
  if (!egl_ctx)
    throw std::runtime_error ("eglCreateContext failed");

  static const int surf_attrs[] = {
    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
    EGL_NONE
  };

  auto egl_surf = eglCreateWindowSurface (egl_disp, config, x11.window, surf_attrs);
  if (!egl_surf)
    throw std::runtime_error ("eglCreateWindowSurface failed");

  ok = eglMakeCurrent (egl_disp, egl_surf, egl_surf, egl_ctx);
  if (!ok)
    throw std::runtime_error ("eglMakeCurrent failed");

  return gl_objects_t { egl_disp, egl_ctx, egl_surf };
}

void redraw (int width, int height, float t) {
  glViewport (0, 0, width, height);

  glClearColor (0, 0, 0, 0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  float r = float (width) / float (height);
  float s = 5.0f;
  float x = r * s;
  float y = s;
  glOrtho (-x, x, -y, y, -10, 10);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (1, 0, 0);
  glRotatef (t * 100.0f, 0.5773f, 0.5773f, 0.5773f);

  float ch = 0.8, cm = 0.4, cl = 0.1;
  glBegin (GL_QUADS);
    glColor3f (ch, cm, cm);
    glVertex3f (1, -1, -1);
    glVertex3f (1,  1, -1);
    glVertex3f (1,  1,  1);
    glVertex3f (1, -1,  1);

    glColor3f (cm, cl, cl);
    glVertex3f (-1, -1, -1);
    glVertex3f (-1, -1,  1);
    glVertex3f (-1,  1,  1);
    glVertex3f (-1,  1, -1);

    glColor3f (cm, ch, cm);
    glVertex3f (-1, 1, -1);
    glVertex3f (-1, 1,  1);
    glVertex3f ( 1, 1,  1);
    glVertex3f ( 1, 1, -1);

    glColor3f (cl, cm, cl);
    glVertex3f (-1, -1, -1);
    glVertex3f ( 1, -1, -1);
    glVertex3f ( 1, -1,  1);
    glVertex3f (-1, -1,  1);

    glColor3f (cm, cm, ch);
    glVertex3f (-1, -1, 1);
    glVertex3f ( 1, -1, 1);
    glVertex3f ( 1,  1, 1);
    glVertex3f (-1,  1, 1);

    glColor3f (cl, cl, cm);
    glVertex3f (-1, -1, -1);
    glVertex3f (-1,  1, -1);
    glVertex3f ( 1,  1, -1);
    glVertex3f ( 1, -1, -1);
  glEnd ();
}

float now () {
  timespec ts = { 0 };
  clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
  return float (ts.tv_sec) + float (ts.tv_nsec / 1000ll) * 0.000001f;
}

int main () {
  xcb_generic_error_t* xcb_err = nullptr;

  x11_objects_t x11 = setup_x (800, 600);
  xcb_flush (x11.xcb_connection);

  gl_objects_t gl = setup_gl (x11);

  glEnable (GL_DEPTH_TEST);

  int width = 1024,
      height = 768;

  const float tick_hz = 50.0f;
  const float dt = 1.0f / tick_hz;

  float t = 0.0f;
  float accum_time = 0.0f;
  float prev_rt = now ();

  while (true) {
    // handle events
    xcb_generic_event_t* ev;
    while ((ev = xcb_poll_for_event (x11.xcb_connection))) {
      switch (XCB_EVENT_RESPONSE_TYPE (ev)) {
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

    auto get_geom_cookie = xcb_get_geometry (x11.xcb_connection, x11.window);
    auto* geom = xcb_get_geometry_reply (x11.xcb_connection, get_geom_cookie, &xcb_err);
    if (geom) {
      width = geom->width;
      height = geom->height;
      free (geom);
    }

    float cur_rt = now ();
    float diff_rt = cur_rt - prev_rt;
    accum_time += diff_rt;
    prev_rt = cur_rt;

    while (accum_time >= dt) {
      accum_time -= dt;
      t += dt;
    }

    redraw (width, height, t + accum_time);
    eglSwapBuffers (gl.egl_display, gl.egl_surface);
  }

exit_loop:;
}

