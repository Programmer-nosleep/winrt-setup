#include "render/skydome_renderer.h"

#include <cmath>
#include <vector>

#include "world/world_scene.h"

namespace {

constexpr float kPi = 3.1415926535f;
// Keep the dome well inside the camera far plane so the sky shader is always
// visible instead of falling back to the black clear color.
constexpr float kRadius = kWorldSkyDomeRadius;
constexpr int kSegments = 32;
constexpr int kRings = 16;

struct SkyVertex {
  float position[3];
};

void BuildSphere(std::vector<SkyVertex> &vertices,
                 std::vector<unsigned int> &indices) {
  vertices.clear();
  indices.clear();

  for (int ring = 0; ring <= kRings; ++ring) {
    const float phi =
        kPi * static_cast<float>(ring) / static_cast<float>(kRings);
    const float sin_phi = std::sin(phi);
    const float cos_phi = std::cos(phi);

    for (int seg = 0; seg <= kSegments; ++seg) {
      const float theta =
          2.0f * kPi * static_cast<float>(seg) / static_cast<float>(kSegments);
      const float sin_theta = std::sin(theta);
      const float cos_theta = std::cos(theta);

      // Z-up sphere: x = r*sin(phi)*cos(theta), y = r*sin(phi)*sin(theta), z =
      // r*cos(phi)
      SkyVertex vertex{};
      vertex.position[0] = kRadius * sin_phi * cos_theta;
      vertex.position[1] = kRadius * sin_phi * sin_theta;
      vertex.position[2] = kRadius * cos_phi;
      vertices.push_back(vertex);
    }
  }

  for (int ring = 0; ring < kRings; ++ring) {
    for (int seg = 0; seg < kSegments; ++seg) {
      const unsigned int current =
          static_cast<unsigned int>(ring * (kSegments + 1) + seg);
      const unsigned int next =
          current + static_cast<unsigned int>(kSegments + 1);

      // Winding order reversed (BackSide rendering — we're inside the sphere)
      indices.push_back(current);
      indices.push_back(current + 1);
      indices.push_back(next);

      indices.push_back(next);
      indices.push_back(current + 1);
      indices.push_back(next + 1);
    }
  }
}

} // namespace

bool SkyDomeRenderer::Initialize(GlApi const &gl,
                                 std::filesystem::path const &shader_directory,
                                 std::string &error) {
  Destroy(gl);

  try {
    if (!shader_program_.LoadFromFiles(gl, shader_directory / "skydome.vert",
                                       shader_directory / "skydome.frag",
                                       error)) {
      return false;
    }
  } catch (std::exception const &exception) {
    error = exception.what();
    return false;
  }

  loc_sky_low_color_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_sky_low_color");
  loc_sky_base_blue_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_sky_base_blue");
  loc_ground_gray_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_ground_gray");
  loc_use_screen_gradient_ =
      gl.GetUniformLocation(shader_program_.Handle(), "u_use_screen_gradient");

  std::vector<SkyVertex> vertices;
  std::vector<unsigned int> indices;
  BuildSphere(vertices, indices);

  gl.GenVertexArrays(1, &vertex_array_);
  gl.GenBuffers(1, &vertex_buffer_);
  gl.GenBuffers(1, &index_buffer_);

  gl.BindVertexArray(vertex_array_);

  gl.BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  gl.BufferData(GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(vertices.size() * sizeof(SkyVertex)),
                vertices.data(), GL_STATIC_DRAW);

  gl.EnableVertexAttribArray(0);
  gl.VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkyVertex), nullptr);

  gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
  gl.BufferData(GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                indices.data(), GL_STATIC_DRAW);

  gl.BindVertexArray(0);
  gl.BindBuffer(GL_ARRAY_BUFFER, 0);

  index_count_ = static_cast<GLsizei>(indices.size());
  return true;
}

void SkyDomeRenderer::Destroy(GlApi const &gl) {
  shader_program_.Destroy(gl);

  if (index_buffer_ != 0) {
    gl.DeleteBuffers(1, &index_buffer_);
    index_buffer_ = 0;
  }
  if (vertex_buffer_ != 0) {
    gl.DeleteBuffers(1, &vertex_buffer_);
    vertex_buffer_ = 0;
  }
  if (vertex_array_ != 0) {
    gl.DeleteVertexArrays(1, &vertex_array_);
    vertex_array_ = 0;
  }

  index_count_ = 0;
}

void SkyDomeRenderer::Render(GlApi const &gl, GLuint view_projection_location,
                             float const *view_projection_matrix,
                             float sky_low_r, float sky_low_g, float sky_low_b,
                             float sky_base_r, float sky_base_g,
                             float sky_base_b, float ground_r, float ground_g,
                             float ground_b, float use_screen_gradient) const {
  if (index_count_ == 0) {
    return;
  }

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_CULL_FACE);

  gl.UseProgram(shader_program_.Handle());

  gl.UniformMatrix4fv(static_cast<GLint>(view_projection_location), 1, GL_FALSE,
                      view_projection_matrix);

  if (loc_sky_low_color_ >= 0) {
    gl.Uniform3f(loc_sky_low_color_, sky_low_r, sky_low_g, sky_low_b);
  }
  if (loc_sky_base_blue_ >= 0) {
    gl.Uniform3f(loc_sky_base_blue_, sky_base_r, sky_base_g, sky_base_b);
  }
  if (loc_ground_gray_ >= 0) {
    gl.Uniform3f(loc_ground_gray_, ground_r, ground_g, ground_b);
  }
  if (loc_use_screen_gradient_ >= 0) {
    gl.Uniform1f(loc_use_screen_gradient_, use_screen_gradient);
  }

  gl.BindVertexArray(vertex_array_);
  glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);
  gl.BindVertexArray(0);

  gl.UseProgram(0);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
}
