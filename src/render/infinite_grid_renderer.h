#pragma once

#include <filesystem>
#include <string>

#include <SDL3/SDL_opengl.h>

#include "render/gl_api.h"
#include "render/shader_program.h"
#include "utils/math3d.h"
#include "world/world_scene.h"

class InfiniteGridRenderer {
public:
  bool Initialize(GlApi const &gl,
                  std::filesystem::path const &shader_directory,
                  std::string &error);

  void Destroy(GlApi const &gl);

  void Render(GlApi const &gl, float const *view_projection_matrix,
              Vec3 const &camera_position, float camera_distance,
              WorldVisualStyle const &style, bool show_axes) const;

private:
  ShaderProgram shader_program_{};
  GLuint vertex_array_ = 0;
  GLuint vertex_buffer_ = 0;

  GLint loc_grid_origin_ = -1;
  GLint loc_grid_extent_ = -1;
  GLint loc_cell_size_ = -1;
  GLint loc_major_step_ = -1;
  GLint loc_fade_start_ = -1;
  GLint loc_fade_end_ = -1;
  GLint loc_minor_color_ = -1;
  GLint loc_major_color_ = -1;
  GLint loc_axis_x_color_ = -1;
  GLint loc_axis_y_color_ = -1;
  GLint loc_axis_emphasis_ = -1;
};
