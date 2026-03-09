#include "render/infinite_grid_renderer.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {

float ComputeGridCellSize(float camera_distance) {
  const float scaled_distance = std::max(0.25f, camera_distance * 0.08f);
  const float exponent = std::floor(std::log2(scaled_distance));
  return std::max(0.25f, std::pow(2.0f, exponent));
}

} // namespace

bool InfiniteGridRenderer::Initialize(GlApi const &gl,
                                      std::filesystem::path const &shader_directory,
                                      std::string &error) {
  Destroy(gl);

  try {
    if (!shader_program_.LoadFromFiles(gl, shader_directory / "infinite_grid.vert",
                                       shader_directory / "infinite_grid.frag",
                                       error)) {
      return false;
    }
  } catch (std::exception const &exception) {
    error = exception.what();
    return false;
  }

  loc_grid_origin_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_grid_origin");
  loc_grid_extent_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_grid_extent");
  loc_cell_size_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_cell_size");
  loc_major_step_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_major_step");
  loc_fade_start_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_fade_start");
  loc_fade_end_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_fade_end");
  loc_minor_color_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_minor_color");
  loc_major_color_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_major_color");
  loc_axis_x_color_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_axis_x_color");
  loc_axis_y_color_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_axis_y_color");
  loc_axis_emphasis_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_axis_emphasis");

  constexpr std::array<float, 12> kPlaneVertices = {
      -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,
      -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
  };

  gl.GenVertexArrays(1, &vertex_array_);
  gl.GenBuffers(1, &vertex_buffer_);

  gl.BindVertexArray(vertex_array_);
  gl.BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  gl.BufferData(GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(kPlaneVertices.size() * sizeof(float)),
                kPlaneVertices.data(), GL_STATIC_DRAW);

  gl.EnableVertexAttribArray(0);
  gl.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);

  gl.BindVertexArray(0);
  gl.BindBuffer(GL_ARRAY_BUFFER, 0);

  return true;
}

void InfiniteGridRenderer::Destroy(GlApi const &gl) {
  shader_program_.Destroy(gl);

  if (vertex_buffer_ != 0) {
    gl.DeleteBuffers(1, &vertex_buffer_);
    vertex_buffer_ = 0;
  }

  if (vertex_array_ != 0) {
    gl.DeleteVertexArrays(1, &vertex_array_);
    vertex_array_ = 0;
  }
}

void InfiniteGridRenderer::Render(GlApi const &gl,
                                  float const *view_projection_matrix,
                                  Vec3 const &camera_position,
                                  float camera_distance,
                                  WorldVisualStyle const &style,
                                  bool show_axes) const {
  if (vertex_array_ == 0) {
    return;
  }

  const float cell_size = ComputeGridCellSize(camera_distance);
  const float major_step = 8.0f;
  const float major_spacing = cell_size * major_step;
  const float dome_ground_radius = kWorldSkyDomeRadius * 0.96f;
  const float extent = dome_ground_radius;
  const float fade_end = dome_ground_radius;
  const float fade_start =
      fade_end * std::clamp(style.horizon_ratio, 0.55f, 0.82f);
  const Vec3 snapped_origin{
      std::round(camera_position.x / major_spacing) * major_spacing,
      std::round(camera_position.y / major_spacing) * major_spacing,
      -0.02f,
  };

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  gl.UseProgram(shader_program_.Handle());
  gl.UniformMatrix4fv(shader_program_.ViewProjectionLocation(), 1, GL_FALSE,
                      view_projection_matrix);

  if (loc_grid_origin_ >= 0) {
    gl.Uniform3f(loc_grid_origin_, snapped_origin.x, snapped_origin.y,
                 snapped_origin.z);
  }
  if (loc_grid_extent_ >= 0) {
    gl.Uniform1f(loc_grid_extent_, extent);
  }
  if (loc_cell_size_ >= 0) {
    gl.Uniform1f(loc_cell_size_, cell_size);
  }
  if (loc_major_step_ >= 0) {
    gl.Uniform1f(loc_major_step_, major_step);
  }
  if (loc_fade_start_ >= 0) {
    gl.Uniform1f(loc_fade_start_, fade_start);
  }
  if (loc_fade_end_ >= 0) {
    gl.Uniform1f(loc_fade_end_, fade_end);
  }
  if (loc_minor_color_ >= 0) {
    gl.Uniform3f(loc_minor_color_, style.grid_minor[0], style.grid_minor[1],
                 style.grid_minor[2]);
  }
  if (loc_major_color_ >= 0) {
    gl.Uniform3f(loc_major_color_, style.grid_major[0], style.grid_major[1],
                 style.grid_major[2]);
  }
  if (loc_axis_x_color_ >= 0) {
    gl.Uniform3f(loc_axis_x_color_, style.axis_x[0], style.axis_x[1],
                 style.axis_x[2]);
  }
  if (loc_axis_y_color_ >= 0) {
    gl.Uniform3f(loc_axis_y_color_, style.axis_y[0], style.axis_y[1],
                 style.axis_y[2]);
  }
  if (loc_axis_emphasis_ >= 0) {
    gl.Uniform1f(loc_axis_emphasis_, show_axes ? 1.0f : 0.0f);
  }

  gl.BindVertexArray(vertex_array_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  gl.BindVertexArray(0);

  gl.UseProgram(0);
  glDepthMask(GL_TRUE);
}
