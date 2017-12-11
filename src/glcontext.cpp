//
// no-dice
//

#include "glcontext.hpp"

#include <Rk/guard.hpp>

#include <string>
#include <iostream>
#include <stdexcept>

#include <epoxy/gl.h>

namespace nd {
  #ifndef NDEBUG
  static void handle_gl_debug (
    GLenum src, GLenum type, uint id, GLenum level,
    GLsizei length, char const* msg,
    void const*)
  {
    std::cerr << "GL: " << msg << "\n";
  }
  #endif

  GLContext GLContext::establish (EGLNativeWindowType nat_win) {
    using namespace std::string_literals;

    auto ok = eglBindAPI (EGL_OPENGL_API);
    if (!ok)
      throw std::runtime_error ("eglBindAPI failed");

    auto egl_disp = eglGetDisplay (EGL_DEFAULT_DISPLAY);//(nat_disp);
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
      EGL_SAMPLE_BUFFERS,    1,
      EGL_SAMPLES,           4,
      EGL_NONE
    };

    EGLConfig config;
    int config_count = 0;
    ok = eglChooseConfig (egl_disp, config_attrs, &config, 1, &config_count);
    if (!ok || config_count == 0)
      throw std::runtime_error ("eglChooseConfig failed");

    static const int context_attrs[] = {
      EGL_CONTEXT_MAJOR_VERSION, 4,
      EGL_CONTEXT_MINOR_VERSION, 3,
    #ifndef NDEBUG
      EGL_CONTEXT_FLAGS_KHR,     EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
    #endif
      EGL_NONE
    };

    auto egl_ctx = eglCreateContext (egl_disp, config, EGL_NO_CONTEXT, context_attrs);
    if (!egl_ctx)
      throw std::runtime_error ("eglCreateContext failed");

    static const int surf_attrs[] = {
      EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
      EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB,
      EGL_NONE
    };

    auto egl_surf = eglCreateWindowSurface (egl_disp, config, nat_win, surf_attrs);
    if (!egl_surf)
      throw std::runtime_error ("eglCreateWindowSurface failed");

    ok = eglMakeCurrent (egl_disp, egl_surf, egl_surf, egl_ctx);
    if (!ok)
      throw std::runtime_error ("eglMakeCurrent failed");

    glEnable (GL_FRAMEBUFFER_SRGB);

    glHint (GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

  //glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth (3.f);

    glEnable (GL_MULTISAMPLE);

    #ifndef NDEBUG
    glDebugMessageCallback (handle_gl_debug, nullptr);
    glEnable (GL_DEBUG_OUTPUT);
    #endif

    #ifdef PROFILING
      eglSwapInterval (egl_disp, 0);
    #else
      eglSwapInterval (egl_disp, 1);
    #endif

    return GLContext (egl_disp, egl_ctx, egl_surf);
  }

  GLContext::~GLContext () {
    eglMakeCurrent (disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate (disp);
  }

  void GLContext::flip () {
    eglSwapBuffers (disp, surf);
  }
}

