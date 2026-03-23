# Metroidvania CMake Setup (vcpkg + fmt)

This project uses CMake presets and vcpkg manifest mode. The repository itself does not require a specific compiler family, but local MinGW setup is recommended for this workspace.

## What The Repository Requires

Required files tracked in git:
- `CMakeLists.txt` (uses `find_package(fmt CONFIG REQUIRED)`)
- `CMakePresets.json` (shared preset config)
- `vcpkg.json` (declares dependency `fmt`)
- `vcpkg-configuration.json` (registry/baseline config)

Required tools:
- CMake
- vcpkg
- Git
- A supported C++ compiler toolchain

## Recommended Setup

MinGW (MSYS2 UCRT64) is the recommended local compiler toolchain for this workspace.

How MinGW fits into this setup:
- `CMakePresets.json` provides shared project-level settings (including vcpkg toolchain usage).
- Your local `CMakeUserPresets.json` selects MinGW compiler by setting `CMAKE_C_COMPILER` and `CMAKE_CXX_COMPILER` to gcc/g++.
- The user preset must select a MinGW-compatible vcpkg triplet (for example, `x64-mingw-static`) so dependencies are built for the same toolchain.

## Recommended Local MinGW User Preset

To setup local machine-specific settings such as a `CMakeUserPresets.json` file needs to be created.
This file specifies, compiler paths, build type presets, the local vcpkg root environment value `VCPKG_ROOT`.

If you use MinGW (MSYS2 UCRT64), create a local `CMakeUserPresets.json` with settings similar to
the following. The debug preset is optional.

```json
{
  "version": 2,
  "configurePresets": [
    {
      "name": "default-release",
      "inherits": "vcpkg",
      "binaryDir": "${sourceDir}/build/release",
      "cacheVariables": {
        "CMAKE_C_COMPILER": "C:/Program Files/msys2/ucrt64/bin/gcc.exe",
        "CMAKE_CXX_COMPILER": "C:/Program Files/msys2/ucrt64/bin/g++.exe",
        "VCPKG_TARGET_TRIPLET": "x64-mingw-static",
        "CMAKE_BUILD_TYPE": "Release"
      },
      "environment": {
        "VCPKG_ROOT": "C:\\Program Files\\vcpkg"
      }
    },
        {
      "name": "default-debug",
      "inherits": "vcpkg",
      "binaryDir": "${sourceDir}/build/debug",
      "cacheVariables": {
        "CMAKE_C_COMPILER": "C:/Program Files/msys2/ucrt64/bin/gcc.exe",
        "CMAKE_CXX_COMPILER": "C:/Program Files/msys2/ucrt64/bin/g++.exe",
        "VCPKG_TARGET_TRIPLET": "x64-mingw-static",
        "CMAKE_BUILD_TYPE": "Debug"
      },
      "environment": {
        "VCPKG_ROOT": "C:\\Program Files\\vcpkg"
      }
    }
  ]
}
```

## Configure And Build 

### VS Code

Before selecting presets, make sure the CMake Tools extension is in preset mode.
Set this in workspace settings (`.vscode/settings.json`):

```json
{
  "cmake.useCMakePresets": "always"
}
```

If this is not enabled, VS Code may stay in Kit workflow and commands like preset selection may not appear.

1. Open Command Palette.
2. Run `CMake: Select Configure Preset`.
3. Choose a preset available on your machine (from tracked presets and any local user presets).
4. Run `CMake: Delete Cache and Reconfigure`.
5. Run `CMake: Build`.

### Terminal

Run with whatever preset names exist locally, for example:

```powershell
cmake --preset default-debug
cmake --build --preset default-debug
```
