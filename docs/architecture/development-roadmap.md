# Development Roadmap

Dokumen ini adalah catatan engineering untuk melanjutkan project `winrt-setup` dan, kalau nanti dibutuhkan, memecahnya menjadi fondasi engine + editor shell.

## Tujuan Dokumen

- menyimpan snapshot state code saat ini
- menjelaskan subsystem yang sudah ada
- mencatat technical debt yang paling penting
- memberi arah refactor jika project diarahkan menjadi engine game / engine editor sendiri

## State Saat Ini

Project ini masih berupa desktop application tunggal dengan karakter editor/CAD shell:

- `SDL3` menangani window, event loop, dan OpenGL context
- `OpenGL 3.3 Core` menangani render 3D
- `Dear ImGui` menangani seluruh shell UI editor
- `C++/WinRT` dipakai untuk membaca info sistem Windows
- scene 3D masih berupa world statis yang dibangun dari `world_scene.cpp`

Belum ada boundary formal antara:

- engine runtime
- editor shell
- asset pipeline
- gameplay framework

Artinya, saat ini project lebih dekat ke `editor prototype` daripada `game engine` yang benar-benar modular.

## Subsystem Yang Sudah Ada

### 1. App Shell

File utama:

- `src/app/main.cpp`
- `src/app/application.h`
- `src/app/application.cpp`

Tanggung jawab:

- bootstrap WinRT apartment
- init SDL + OpenGL + ImGui
- event loop utama
- shortcut keyboard
- layout UI editor
- sinkronisasi state viewport/tool/theme

Catatan:

- `Application` masih memegang terlalu banyak tanggung jawab
- kalau project tumbuh, file ini perlu dipecah menjadi controller yang lebih kecil

### 2. UI Layer

File utama:

- `src/ui/workspace_model.*`
- `src/ui/workspace_widgets.*`
- `src/style/ui_style.*`
- `src/ui/svg_icon_cache.*`

Tanggung jawab:

- model konten sidebar/inspector
- widget custom ImGui
- typography, spacing, metrics
- caching icon SVG

Catatan:

- layer ini sudah lumayan cocok dijadikan `editor module`
- sebaiknya jangan dicampur langsung ke runtime engine di masa depan

### 3. Render Layer

File utama:

- `src/render/gl_api.*`
- `src/render/shader_program.*`
- `src/render/world_renderer.*`
- `src/render/skydome_renderer.*`
- `src/render/infinite_grid_renderer.*`

Tanggung jawab:

- load OpenGL function pointer
- compile/link shader
- render pass skydome
- render pass infinite grid
- render pass world geometry

Urutan pass saat ini:

1. clear framebuffer netral
2. render skydome shader
3. render infinite grid shader
4. render massing / glass / outlines / section lines / axes / guide cube

Catatan:

- ini sudah mulai membentuk `render pipeline` sederhana
- masih belum ada render graph, material abstraction, mesh abstraction, atau resource lifetime manager

### 4. World/Scene Layer

File utama:

- `src/world/world_scene.*`
- `src/world/orbit_camera.*`
- `src/utils/math3d.h`

Tanggung jawab:

- style visual dunia
- data vertex scene statis
- kamera orbit/pan/zoom
- helper math dasar

Catatan:

- `WorldScene` saat ini masih hard-coded
- belum ada entity system, transform hierarchy, scene serialization, atau resource database

## Technical Debt Yang Penting

### Application Masih Gemuk

`Application` saat ini menggabungkan:

- shell UI
- input routing
- camera mode switching
- panel state
- render orchestration
- shortcut handling

Refactor yang disarankan:

- `EditorShell`
- `ViewportController`
- `WorkspaceLayoutController`
- `InputRouter`
- `EditorState`

### Scene Masih Hard-Coded

Saat ini geometry utama dibentuk manual di `world_scene.cpp`.

Implikasi:

- sulit untuk scaling
- tidak ada format scene
- belum ada asset import/export
- gameplay layer belum bisa lahir dari struktur ini

### Renderer Masih Immediate dan Flat

Saat ini renderer masih sederhana:

- belum ada mesh object reusable
- belum ada uniform buffer
- belum ada texture/material system yang proper
- belum ada frame graph
- belum ada batching policy formal

### Editor dan Engine Belum Dipisah

Kalau target akhirnya engine sendiri, ini penting:

- engine sebaiknya tidak bergantung pada ImGui
- engine harus bisa hidup tanpa editor shell
- editor cukup menjadi client/tooling di atas engine API

## Arah Jika Mau Dibuat Menjadi Engine Sendiri

Saran paling pragmatis: jangan ubah repo ini langsung menjadi "engine besar" dalam satu langkah. Pecah bertahap.

### Tahap 1. Bersihkan Boundary

Target:

- pisahkan `editor shell` dari `engine-ish code`

Pecahan minimal:

- `src/editor/`
- `src/engine/render/`
- `src/engine/scene/`
- `src/engine/math/`
- `src/engine/platform/`

Yang dipindahkan lebih dulu:

- `world_renderer.*`
- `skydome_renderer.*`
- `infinite_grid_renderer.*`
- `orbit_camera.*`
- `math3d.h`

### Tahap 2. Bentuk Scene Runtime

Tambahkan abstraction berikut:

- `Entity`
- `Transform`
- `Mesh`
- `Material`
- `Camera`
- `Light`
- `Scene`

Target tahap ini:

- scene tidak lagi hard-coded dari satu fungsi
- object bisa di-spawn, dihapus, diserialisasi

### Tahap 3. Bentuk Resource System

Tambahkan:

- shader registry
- mesh registry
- texture/material registry
- hot reload yang lebih formal

Kalau tidak ada resource system, editor akan cepat berantakan.

### Tahap 4. Pisahkan Editor dari Runtime

Setelah subsystem dasar cukup:

- editor memanggil engine API
- engine bisa dipakai oleh executable lain tanpa ImGui

Ini langkah penting kalau tujuannya memang engine sendiri.

### Tahap 5. Tambah Gameplay/Simulation Layer

Kalau project bergerak dari CAD/editor shell ke game engine, layer berikut baru layak masuk:

- input mapping system
- component/system runtime
- physics integration
- animation system
- scripting layer
- audio layer
- asset cooking

## Rekomendasi Arsitektur Engine

Kalau kamu benar-benar mau bikin engine sendiri, struktur paling aman untuk skala kecil-menengah adalah:

### Core

- memory helpers
- logging
- time
- UUID / handles
- assertions

### Platform

- window
- input
- file system
- thread abstraction

### Math

- vector
- matrix
- quaternion
- bounds / ray / frustum

### Render

- device/context
- shader
- buffer
- texture
- material
- mesh
- render passes

### Scene

- entity
- transform
- scene graph
- camera
- lights

### Editor

- docking/layout
- outliner
- inspector
- asset browser
- viewport tools

## Next Step Yang Paling Masuk Akal Di Repo Ini

Kalau goal jangka dekat masih editor/workspace:

1. pecah `Application` menjadi controller kecil
2. buat `EditorState` terpusat
3. buat `SceneObject` runtime sederhana
4. ubah `world_scene.cpp` dari hard-coded mesh ke scene/model abstraction
5. buat serializer JSON sederhana untuk scene

Kalau goal jangka dekat mulai ke engine:

1. buat namespace/module `engine`
2. pindahkan renderer, camera, math ke sana
3. buat `Scene` dan `Entity` abstraction
4. buat editor memanggil engine API, bukan state world langsung

## Hal Yang Sudah Bagus dan Layak Dipertahankan

- shader dipisah ke `res/shaders`
- icon dipisah dan dicache
- ada pembagian model UI vs widget UI
- ada render pass terpisah untuk skydome dan infinite grid
- camera logic tidak bercampur ke shader code

Itu semua sudah cocok jadi fondasi refactor yang sehat.

## Catatan Penutup

Kalau project ini akan berkembang menjadi engine, mindset yang paling aman adalah:

- treat repo ini sebagai `editor prototype + rendering lab`
- ekstrak engine secara bertahap dari subsystem yang sudah stabil
- jangan mulai dari fitur terlalu banyak
- prioritaskan boundary, ownership, dan lifecycle object

Engine yang kuat biasanya lahir dari pemisahan tanggung jawab yang disiplin, bukan dari banyaknya fitur di awal.
