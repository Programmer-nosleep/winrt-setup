#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

struct GlApi {
  bool Load(std::string &error);

  PFNGLATTACHSHADERPROC AttachShader = nullptr;
  PFNGLBINDBUFFERPROC BindBuffer = nullptr;
  PFNGLBINDVERTEXARRAYPROC BindVertexArray = nullptr;
  PFNGLBUFFERDATAPROC BufferData = nullptr;
  PFNGLCOMPILESHADERPROC CompileShader = nullptr;
  PFNGLCREATEPROGRAMPROC CreateProgram = nullptr;
  PFNGLCREATESHADERPROC CreateShader = nullptr;
  PFNGLDELETEBUFFERSPROC DeleteBuffers = nullptr;
  PFNGLDELETEPROGRAMPROC DeleteProgram = nullptr;
  PFNGLDELETESHADERPROC DeleteShader = nullptr;
  PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays = nullptr;
  PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray = nullptr;
  PFNGLGENBUFFERSPROC GenBuffers = nullptr;
  PFNGLGENVERTEXARRAYSPROC GenVertexArrays = nullptr;
  PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog = nullptr;
  PFNGLGETPROGRAMIVPROC GetProgramiv = nullptr;
  PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog = nullptr;
  PFNGLGETSHADERIVPROC GetShaderiv = nullptr;
  PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation = nullptr;
  PFNGLLINKPROGRAMPROC LinkProgram = nullptr;
  PFNGLSHADERSOURCEPROC ShaderSource = nullptr;
  PFNGLUNIFORM1FPROC Uniform1f = nullptr;
  PFNGLUNIFORM3FPROC Uniform3f = nullptr;
  PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv = nullptr;
  PFNGLUSEPROGRAMPROC UseProgram = nullptr;
  PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer = nullptr;
};
