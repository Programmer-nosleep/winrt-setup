# Shaders

## Lokasi

Shader runtime berada di:

- `res/shaders/skydome.vert`
- `res/shaders/skydome.frag`
- `res/shaders/infinite_grid.vert`
- `res/shaders/infinite_grid.frag`
- `res/shaders/world.vert`
- `res/shaders/world.frag`

## Render Pass

### Skydome

- renderer: `SkyDomeRenderer`
- tujuan: gradient langit + ground horizon
- catatan: sky sekarang datang dari shader, bukan background SDL/ImGui

### Infinite Grid

- renderer: `InfiniteGridRenderer`
- tujuan: procedural endless grid di plane ground
- karakter:
  - ukuran cell adaptif terhadap jarak kamera
  - fade ke tepi
  - major lines
  - axis emphasis saat `show_axes` aktif

### World Geometry

- renderer: `WorldRenderer`
- tujuan: massing mesh, glass, floor outlines, section lines, axes, guide cube

## Uniform

### `u_view_projection`

Matrix gabungan projection dan view dari `OrbitCamera`.

Uniform tambahan ada di shader skydome dan infinite grid, misalnya:

- warna sky low / sky top / ground
- origin dan extent grid
- ukuran cell dan major step
- fade range grid

## Vertex Format World

Renderer mengirim data vertex berupa:

- `position[3]`
- `color[3]`

Format ini didefinisikan di `WorldVertex` pada `src/world/world_scene.h`.

## Reload

Renderer bisa di-reload dari aplikasi tanpa rebuild executable. Ini berguna saat shader sedang diubah.

## Catatan Pengembangan

- shader sekarang sudah dibagi per pass, jadi lebih mudah berkembang ke pipeline yang lebih serius
- langkah berikut yang wajar adalah memisahkan resource management shader/material dari `WorldRenderer`
- kalau nanti masuk ke material system, mesh asset, atau post-process, dokumentasi ini perlu diperluas lagi
