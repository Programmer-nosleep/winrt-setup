# Project Structure

## Root

- `CMakeLists.txt`: setup project, dependency, WinRT projection, copy resource
- `CMakePresets.json`: preset configure/build
- `cmake/CPM.cmake`: bootstrap CPM
- `res/`: runtime resource
- `src/`: source code
- `docs/`: dokumentasi project

## Source Tree

### `src/app`

- `main.cpp`: entry point
- `app_types.h/.cpp`: enum dan label UI/tool
- `application.h/.cpp`: shell UI, event handling, lifecycle aplikasi

### `src/event`

- `input.h`: shortcut dan label keyboard

### `src/ui`

- `workspace_model.h/.cpp`: data model untuk rail, browser, inspector, dan viewport overlay
- `workspace_widgets.h/.cpp`: komponen ImGui reusable untuk chip, toggle, browser rows, dan overlay viewport

### `src/platform`

- `winrt_info.h/.cpp`: baca informasi sistem via WinRT

### `src/style`

- `ui_style.h/.cpp`: theme ImGui, font UI, ukuran shell, dan state style tombol aktif

### `src/render`

- `gl_api.h/.cpp`: OpenGL function loader berbasis `SDL_GL_GetProcAddress`
- `shader_program.h/.cpp`: loading, compile, link shader
- `skydome_renderer.h/.cpp`: render skydome shader
- `infinite_grid_renderer.h/.cpp`: render procedural infinite grid
- `world_renderer.h/.cpp`: orchestrator render world

### `src/utils`

- `math3d.h`: vector dan matrix helper sederhana

### `src/world`

- `world_scene.h/.cpp`: data world architectural dan vertex range tiap elemen
- `orbit_camera.h/.cpp`: kontrol kamera orbit/pan/zoom dan preset view termasuk back/left

## Resource Tree

### `res/shaders`

- `skydome.vert/.frag`: shader skydome
- `infinite_grid.vert/.frag`: shader infinite grid
- `world.vert`: vertex shader world
- `world.frag`: fragment shader world

Shader dipisahkan dari source supaya mudah diganti tanpa mengedit file C++.
