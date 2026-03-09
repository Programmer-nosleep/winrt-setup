# WinRT Sketch World

Shell desktop CAD-style berbasis `CMake`, `C++/WinRT`, `SDL3`, `OpenGL`, `ImGui`, dan `fmt`.

Dokumentasi utama sekarang dipisah ke folder [docs/](docs/README.md).

## Quick Start

```powershell
cmake --preset vs2026-x64
cmake --build --preset debug
```

Output executable:

- `build/vs2026-x64/Debug/winrt_hello.exe`

## Dokumentasi

- [docs/README.md](docs/README.md): index dokumentasi
- [docs/getting-started/build-and-run.md](docs/getting-started/build-and-run.md): build dan run
- [docs/architecture/overview.md](docs/architecture/overview.md): gambaran sistem
- [docs/architecture/project-structure.md](docs/architecture/project-structure.md): struktur folder
- [docs/architecture/development-roadmap.md](docs/architecture/development-roadmap.md): catatan engineering dan arah menuju engine
- [docs/reference/keyboard-shortcuts.md](docs/reference/keyboard-shortcuts.md): shortcut aktif
- [docs/reference/shaders.md](docs/reference/shaders.md): shader dan render path
