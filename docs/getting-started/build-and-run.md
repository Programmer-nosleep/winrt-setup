# Build And Run

## Prasyarat

- Windows
- Visual Studio dengan workload C++
- Windows SDK yang menyediakan `cppwinrt.exe`
- Internet saat configure pertama untuk download dependency CPM

## Dependency yang dipakai

- `fmt`
- `SDL3`
- `Dear ImGui`
- `C++/WinRT`

Semua dependency non-SDK di-resolve dari `CPM.cmake`.

## Configure

Preset utama:

```powershell
cmake --preset vs2026-x64
```

Manual:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
```

## Build

Preset:

```powershell
cmake --build --preset debug
```

Manual:

```powershell
cmake --build build --config Debug
```

## Run

Executable debug ada di:

```text
build/vs2026-x64/Debug/winrt_hello.exe
```

Saat build selesai, folder `res/` akan dicopy ke output directory agar shader bisa langsung ditemukan.

## Catatan Build

- `CMakeLists.txt` mencari `cppwinrt.exe` dari Windows SDK yang terpasang.
- Header projection WinRT digenerate ke `build/.../generated/cppwinrt`.
- Runtime DLL dari dependency juga dicopy ke folder output executable.
