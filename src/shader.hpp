//
// no-dice
//

#pragma once

#include "types.hpp"

#include <exception>

#include <epoxy/gl.h>

namespace nd {
  class ShaderCompileError : public std::exception {
    std::string msg;

  public:
    ShaderCompileError (char const* log);
    char const* what () const noexcept override;
  };

  enum class ShaderType : u32 {
    vertex   = GL_VERTEX_SHADER,
    fragment = GL_FRAGMENT_SHADER
  };

  class Shader {
    u32 glname = 0;

  public:
    Shader () = default;
    explicit Shader (u32 name) : glname (name) { }

    Shader (Shader&& other) : glname (other.reset ()) { }

    Shader& operator = (Shader&& other) {
      std::swap (glname, other.glname);
      return *this;
    }

    Shader (Shader const&) = delete;
    Shader& operator = (Shader const&) = delete;

    ~Shader ();

    static Shader compile (StrRef source, ShaderType type);

    u32 name () const { return glname; }

    u32 reset () {
      u32 result = 0;
      std::swap (result, glname);
      return result;
    }
  };

  class ShaderProgram {
    u32 glname = 0;

  public:
    ShaderProgram () = default;
    explicit ShaderProgram (u32 name) : glname (name) { }

    ShaderProgram (ShaderProgram&& other) :
      glname (other.glname)
    { other.glname = 0; }

    ShaderProgram& operator = (ShaderProgram&& other) {
      std::swap (glname, other.glname);
      return *this;
    }

    ShaderProgram (ShaderProgram const&) = delete;
    ShaderProgram& operator = (ShaderProgram const&) = delete;

    ~ShaderProgram ();

    u32 name () const { return glname; }

    void use ();
  };

  class ShaderLinker {
    u32 progname;

  public:
    ShaderLinker ();
    ~ShaderLinker ();

    void add_shader (Shader);
    void fix_attrib (char const* name, u32 location);

    ShaderLinker& operator << (Shader s) {
      add_shader (std::move (s));
      return *this;
    }

    ShaderProgram link ();
  };

  Shader load_shader (StrRef path, ShaderType type);

  namespace {
    template<typename... Ss>
    inline ShaderProgram link (Ss... ss) {
      ShaderLinker ln;
      (ln << ... << std::move (ss));
      return ln.link ();
    }
  }
}

