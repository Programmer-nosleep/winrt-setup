# Architecture Overview

Project ini adalah shell desktop ringan dengan gaya aplikasi CAD custom.

## Stack

- `SDL3`: window, OpenGL context, event loop
- `OpenGL 3.3 Core`: renderer world
- `Dear ImGui`: shell UI, menu bar, command bar, inspector, modal help
- `C++/WinRT`: pembacaan informasi sistem Windows Runtime
- `fmt`: formatting string dan status text

## Alur Runtime

1. `src/app/main.cpp` menginisialisasi apartment `C++/WinRT`.
2. `Application` membuat window SDL dan OpenGL context.
3. `GlApi` memuat function pointer OpenGL yang dipakai renderer.
4. `WorldRenderer` memuat shader dari `res/shaders` dan mengupload vertex scene.
5. `Application` menggambar shell UI ImGui lalu merender world 3D.
6. `WinRtSnapshot` dipakai untuk menampilkan system info di menu/help modal.

## UI Shell

Shell saat ini punya beberapa layer:

- rail kiri + project browser
- command bar melayang
- inspector panel modular
- bottom dock / settings bubble
- help modal

Data UI sekarang dipisahkan ke:

- `src/app/app_types.*` untuk enum dan label tool/theme
- `src/ui/workspace_model.*` untuk model konten panel
- `src/ui/workspace_widgets.*` untuk widget ImGui yang reusable

Tujuannya adalah memberi rasa aplikasi CAD/BIM yang lebih dekat ke workspace arch, tapi tetap modular untuk dikembangkan di C++.

## Render Pipeline

Render path saat ini dibagi menjadi beberapa pass:

1. clear framebuffer netral
2. skydome shader pass
3. infinite grid shader pass
4. world geometry pass
5. ImGui pass

Renderer yang aktif sekarang:

- `WorldRenderer`: orchestrator render utama
- `SkyDomeRenderer`: sky gradient berbasis shader
- `InfiniteGridRenderer`: grid procedural berbasis shader

Ini berarti grid tidak lagi digambar sebagai line mesh statis, dan sky tidak lagi bergantung pada warna background SDL/ImGui.

## World Scene

Scene awal berisi:

- architectural massing dua level
- glass band dan floor outlines
- section guides
- optional guide box
- infinite grid procedural
- skydome procedural

Massing, glass, floor outlines, section lines, axes, dan guide cube punya vertex range sendiri agar bisa di-toggle atau di-render terpisah tanpa regenerate seluruh scene. Infinite grid dan skydome dirender sebagai pass shader terpisah.

## Input Model

- mouse untuk orbit, pan, zoom
- keyboard shortcut untuk kamera, panel, dan display toggle
- konstanta shortcut dipusatkan di `src/event/input.h`

## Catatan Arsitektur

Boundary engine vs editor belum formal. Saat ini project masih paling tepat dibaca sebagai:

- editor shell
- renderer prototype
- scene prototype

Kalau nanti diarahkan ke engine sendiri, referensi utamanya ada di [Development Roadmap](development-roadmap.md).
