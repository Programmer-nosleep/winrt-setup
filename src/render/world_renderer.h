#pragma once

#include <filesystem>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "render/gl_api.h"
#include "render/infinite_grid_renderer.h"
#include "render/shader_program.h"
#include "render/skydome_renderer.h"
#include "world/orbit_camera.h"
#include "world/world_scene.h"

class WorldRenderer {
public:
  bool Initialize(GlApi const &gl,
                  std::filesystem::path const &shader_directory,
                  WorldScene const &scene, std::string &error);

  void Destroy(GlApi const &gl);
  void Render(GlApi const &gl, OrbitCamera const &camera,
              WorldScene const &scene, WorldVisualStyle const &style, int width,
              int height, SDL_Rect const &viewport_pixels, bool show_grid, bool show_axes,
              bool show_guide_cube) const;

private:
  ShaderProgram shader_program_{};
  InfiniteGridRenderer infinite_grid_renderer_{};
  SkyDomeRenderer skydome_renderer_{};
  GLuint vertex_array_ = 0;
  GLuint vertex_buffer_ = 0;
  GLsizei vertex_count_ = 0;
};
