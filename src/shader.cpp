//
// no-dice
//

#include "shader.hpp"

#include <Rk/file_stream.hpp>

#include <exception>
#include <string>

#include <epoxy/gl.h>

namespace nd {
  ShaderCompileError::ShaderCompileError (char const* log) {
    msg = "Errors compiling shader:\n";
    msg.append (log);
  }

  char const* ShaderCompileError::what () const noexcept {
    return msg.c_str ();
  }

  Shader::~Shader () {
    glDeleteShader (glname);
  }

  Shader Shader::compile (StrRef source, ShaderType type) {
    u32 glname = glCreateShader ((u32) type);
    char const* data = source.data ();
    i32  const  length = source.length ();
    glShaderSource (glname, 1, &data, &length);
    glCompileShader (glname);

    if (int stat; glGetShaderiv (glname, GL_COMPILE_STATUS, &stat), !stat) {
      int length;
      glGetShaderiv (glname, GL_INFO_LOG_LENGTH, &length);
      auto buffer = std::make_unique<char[]> (length);
      glGetShaderInfoLog (glname, length, nullptr, buffer.get ());
      throw ShaderCompileError (buffer.get ());
    }

    return Shader (glname);
  }

  ShaderProgram::~ShaderProgram () {
    glDeleteProgram (glname);
  }

  void ShaderProgram::use () {
    glUseProgram (glname);
  }

  ShaderLinker::ShaderLinker () :
    progname (0)
  { }

  ShaderLinker::~ShaderLinker () {
    if (progname != 0)
      glDeleteProgram (progname);
  }

  void ShaderLinker::add_shader (Shader shader) {
    if (progname == 0)
      progname = glCreateProgram ();
    glAttachShader (progname, shader.reset ());
  }

  void ShaderLinker::fix_attrib (char const* name, u32 location) {
    glBindAttribLocation (progname, location, name);
  }

  ShaderProgram ShaderLinker::link () {
    glLinkProgram (progname);
    ShaderProgram prog (progname);
    progname = 0;
    return prog;
  }

  Shader load_shader (StrRef path, ShaderType type) try {
    Rk::fio::InStream file (path);
    auto size = file.size ();
    auto buffer = std::make_unique<char[]> (size);
    file.read (buffer.get (), size);
    return Shader::compile (StrRef (buffer.get (), size), type);
  }
  catch (...) {
    std::throw_with_nested (std::runtime_error (to_string (path)));
  }
}

