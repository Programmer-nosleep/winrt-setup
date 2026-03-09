#include "render/gl_api.h"

#include <array>

namespace {

template <typename T> bool LoadFunction(T &out, char const *name) {
  out = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
  return out != nullptr;
}

} // namespace

bool GlApi::Load(std::string &error) {
  struct RequiredFunction {
    char const *name;
    bool (*assign)(GlApi &);
  };

  static constexpr std::array<RequiredFunction, 24> kFunctions = {{
      {"glAttachShader",
       [](GlApi &api) {
         return LoadFunction(api.AttachShader, "glAttachShader");
       }},
      {"glBindBuffer",
       [](GlApi &api) { return LoadFunction(api.BindBuffer, "glBindBuffer"); }},
      {"glBindVertexArray",
       [](GlApi &api) {
         return LoadFunction(api.BindVertexArray, "glBindVertexArray");
       }},
      {"glBufferData",
       [](GlApi &api) { return LoadFunction(api.BufferData, "glBufferData"); }},
      {"glCompileShader",
       [](GlApi &api) {
         return LoadFunction(api.CompileShader, "glCompileShader");
       }},
      {"glCreateProgram",
       [](GlApi &api) {
         return LoadFunction(api.CreateProgram, "glCreateProgram");
       }},
      {"glCreateShader",
       [](GlApi &api) {
         return LoadFunction(api.CreateShader, "glCreateShader");
       }},
      {"glDeleteBuffers",
       [](GlApi &api) {
         return LoadFunction(api.DeleteBuffers, "glDeleteBuffers");
       }},
      {"glDeleteProgram",
       [](GlApi &api) {
         return LoadFunction(api.DeleteProgram, "glDeleteProgram");
       }},
      {"glDeleteShader",
       [](GlApi &api) {
         return LoadFunction(api.DeleteShader, "glDeleteShader");
       }},
      {"glDeleteVertexArrays",
       [](GlApi &api) {
         return LoadFunction(api.DeleteVertexArrays, "glDeleteVertexArrays");
       }},
      {"glEnableVertexAttribArray",
       [](GlApi &api) {
         return LoadFunction(api.EnableVertexAttribArray,
                             "glEnableVertexAttribArray");
       }},
      {"glGenBuffers",
       [](GlApi &api) { return LoadFunction(api.GenBuffers, "glGenBuffers"); }},
      {"glGenVertexArrays",
       [](GlApi &api) {
         return LoadFunction(api.GenVertexArrays, "glGenVertexArrays");
       }},
      {"glGetProgramInfoLog",
       [](GlApi &api) {
         return LoadFunction(api.GetProgramInfoLog, "glGetProgramInfoLog");
       }},
      {"glGetProgramiv",
       [](GlApi &api) {
         return LoadFunction(api.GetProgramiv, "glGetProgramiv");
       }},
      {"glGetShaderInfoLog",
       [](GlApi &api) {
         return LoadFunction(api.GetShaderInfoLog, "glGetShaderInfoLog");
       }},
      {"glGetShaderiv",
       [](GlApi &api) {
         return LoadFunction(api.GetShaderiv, "glGetShaderiv");
       }},
      {"glGetUniformLocation",
       [](GlApi &api) {
         return LoadFunction(api.GetUniformLocation, "glGetUniformLocation");
       }},
      {"glLinkProgram",
       [](GlApi &api) {
         return LoadFunction(api.LinkProgram, "glLinkProgram");
       }},
      {"glShaderSource",
       [](GlApi &api) {
         return LoadFunction(api.ShaderSource, "glShaderSource");
       }},
      {"glUniform1f",
       [](GlApi &api) { return LoadFunction(api.Uniform1f, "glUniform1f"); }},
      {"glUniform3f",
       [](GlApi &api) { return LoadFunction(api.Uniform3f, "glUniform3f"); }},
      {"glUniformMatrix4fv",
       [](GlApi &api) {
         return LoadFunction(api.UniformMatrix4fv, "glUniformMatrix4fv");
       }},
  }};

  for (auto const &function : kFunctions) {
    if (!function.assign(*this)) {
      error = function.name;
      return false;
    }
  }

  if (!LoadFunction(UseProgram, "glUseProgram") ||
      !LoadFunction(VertexAttribPointer, "glVertexAttribPointer")) {
    error = "glUseProgram/glVertexAttribPointer";
    return false;
  }

  return true;
}
