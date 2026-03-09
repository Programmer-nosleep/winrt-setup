#pragma once

#include <filesystem>
#include <string>

#include <SDL3/SDL_opengl.h>

#include "render/gl_api.h"
#include "render/shader_program.h"

class SkyDomeRenderer {
public:
  bool Initialize(GlApi const &gl,
                  std::filesystem::path const &shader_directory,
                  std::string &error);

  void Destroy(GlApi const &gl);

  void Render(GlApi const &gl, GLuint view_projection_location,
              float const *view_projection_matrix, float sky_low_r,
              float sky_low_g, float sky_low_b, float sky_base_r,
              float sky_base_g, float sky_base_b, float ground_r,
              float ground_g, float ground_b, float use_screen_gradient) const;

  [[nodiscard]] ShaderProgram const &Shader() const { return shader_program_; }

private:
  ShaderProgram shader_program_{};
  GLuint vertex_array_ = 0;
  GLuint vertex_buffer_ = 0;
  GLuint index_buffer_ = 0;
  GLsizei index_count_ = 0;

  GLint loc_sky_low_color_ = -1;
  GLint loc_sky_base_blue_ = -1;
  GLint loc_ground_gray_ = -1;
  GLint loc_use_screen_gradient_ = -1;
};
