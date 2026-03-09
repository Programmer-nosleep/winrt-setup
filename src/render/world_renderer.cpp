#include "render/world_renderer.h"

#include <cstddef>
#include <exception>

#include "utils/math3d.h"

bool WorldRenderer::Initialize(GlApi const &gl,
                               std::filesystem::path const &shader_directory,
                               WorldScene const &scene, std::string &error) {
  Destroy(gl);

  try {
    if (!shader_program_.LoadFromFiles(gl, shader_directory / "world.vert",
                                       shader_directory / "world.frag",
                                       error)) {
      return false;
    }

    if (!infinite_grid_renderer_.Initialize(gl, shader_directory, error)) {
      return false;
    }

    if (!skydome_renderer_.Initialize(gl, shader_directory, error)) {
      return false;
    }
  } catch (std::exception const &exception) {
    error = exception.what();
    return false;
  }

  gl.GenVertexArrays(1, &vertex_array_);
  gl.GenBuffers(1, &vertex_buffer_);
  gl.BindVertexArray(vertex_array_);
  gl.BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  gl.BufferData(
      GL_ARRAY_BUFFER,
      static_cast<GLsizeiptr>(scene.vertices.size() * sizeof(WorldVertex)),
      scene.vertices.data(), GL_STATIC_DRAW);

  gl.EnableVertexAttribArray(0);
  gl.VertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
      reinterpret_cast<void const *>(offsetof(WorldVertex, position)));

  gl.EnableVertexAttribArray(1);
  gl.VertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
      reinterpret_cast<void const *>(offsetof(WorldVertex, color)));

  gl.BindBuffer(GL_ARRAY_BUFFER, 0);
  gl.BindVertexArray(0);

  vertex_count_ = static_cast<GLsizei>(scene.vertices.size());
  return true;
}

void WorldRenderer::Destroy(GlApi const &gl) {
  infinite_grid_renderer_.Destroy(gl);
  skydome_renderer_.Destroy(gl);
  shader_program_.Destroy(gl);

  if (vertex_buffer_ != 0) {
    gl.DeleteBuffers(1, &vertex_buffer_);
    vertex_buffer_ = 0;
  }
  if (vertex_array_ != 0) {
    gl.DeleteVertexArrays(1, &vertex_array_);
    vertex_array_ = 0;
  }

  vertex_count_ = 0;
}

void WorldRenderer::Render(GlApi const &gl, OrbitCamera const &camera,
                           WorldScene const &scene,
                           WorldVisualStyle const &style, int width, int height,
                           SDL_Rect const &viewport_pixels, bool show_grid, bool show_axes,
                           bool show_guide_cube) const {
  if (width <= 0 || height <= 0 || vertex_count_ == 0 || viewport_pixels.w <= 0 ||
      viewport_pixels.h <= 0) {
    return;
  }

  glEnable(GL_SCISSOR_TEST);
  glViewport(viewport_pixels.x, viewport_pixels.y, viewport_pixels.w,
             viewport_pixels.h);
  glScissor(viewport_pixels.x, viewport_pixels.y, viewport_pixels.w,
            viewport_pixels.h);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const float aspect_ratio = static_cast<float>(viewport_pixels.w) /
                             static_cast<float>(viewport_pixels.h);
  const Mat4 projection = camera.ProjectionMatrix(aspect_ratio);
  const Mat4 view = camera.ViewMatrix();
  const Mat4 view_projection = Multiply(projection, view);
  Mat4 sky_view = view;
  sky_view.values[12] = 0.0f;
  sky_view.values[13] = 0.0f;
  sky_view.values[14] = 0.0f;
  const Mat4 sky_view_projection = Multiply(projection, sky_view);

  // Render skydome (replaces old scissor-based ground coloring)
  skydome_renderer_.Render(
      gl,
      static_cast<GLuint>(skydome_renderer_.Shader().ViewProjectionLocation()),
      sky_view_projection.Data(), style.sky_color[0], style.sky_color[1],
      style.sky_color[2], style.sky_top_color[0], style.sky_top_color[1],
      style.sky_top_color[2], style.ground_color[0], style.ground_color[1],
      style.ground_color[2], 0.0f);

  if (show_grid) {
    infinite_grid_renderer_.Render(gl, view_projection.Data(), camera.Position(),
                                   camera.Distance(), style, show_axes);
  }

  gl.UseProgram(shader_program_.Handle());
  gl.UniformMatrix4fv(shader_program_.ViewProjectionLocation(), 1, GL_FALSE,
                      view_projection.Data());

  gl.BindVertexArray(vertex_array_);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f);
  if (scene.massing_mesh.count > 0) {
    glDrawArrays(GL_TRIANGLES, scene.massing_mesh.first, scene.massing_mesh.count);
  }
  if (scene.glass_mesh.count > 0) {
    glDrawArrays(GL_TRIANGLES, scene.glass_mesh.first, scene.glass_mesh.count);
  }
  glDisable(GL_POLYGON_OFFSET_FILL);
  if (scene.floor_outlines.count > 0) {
    glDrawArrays(GL_LINES, scene.floor_outlines.first, scene.floor_outlines.count);
  }
  if (scene.section_lines.count > 0) {
    glDrawArrays(GL_LINES, scene.section_lines.first, scene.section_lines.count);
  }
  if (show_axes && scene.axes.count > 0) {
    glDrawArrays(GL_LINES, scene.axes.first, scene.axes.count);
  }
  if (show_guide_cube && scene.guide_cube.count > 0) {
    glDrawArrays(GL_LINES, scene.guide_cube.first, scene.guide_cube.count);
  }
  gl.BindVertexArray(0);
  gl.UseProgram(0);
  glDisable(GL_SCISSOR_TEST);
  glViewport(0, 0, width, height);
}
