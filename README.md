# CodeHub

CodeHub is a C++20 desktop application for writing and running Python and C++ code. Its interface font is bundled with the project, so no system font installation is required.

## Requirements

- Windows
- CMake 3.20 or newer
- A C++20 compiler (the build instructions below use MSYS2 UCRT64 and GCC)
- Python 3, if you want to run Python submissions
- MSYS2 UCRT64 GCC, if you want to compile C++ submissions

The application looks for MSYS2 under `C:\msys64` by default. If MSYS2 is installed elsewhere, set the `MSYS2_ROOT` environment variable to its installation directory. Python must be installed under `%LOCALAPPDATA%\Programs\Python` or available on `PATH`.

## Get the source

Clone the repository with its GLFW dependency:

```powershell
git clone --recurse-submodules https://github.com/Maks-p09/codehub.git
cd codehub
```

If you already cloned without `--recurse-submodules`, run:

```powershell
git submodule update --init --recursive
```

## Build and install

Install CMake and Ninja, then install GCC from an MSYS2 UCRT64 terminal:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

In PowerShell, configure and build:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe
cmake --build build
cmake --install build --prefix dist
```

The install directory contains `chekker_comp.exe`, the bundled font and its license, and the MinGW runtime DLL required by the application.

**Note:** Submitted code is run as a local process and is not sandboxed. Only run code you trust.
