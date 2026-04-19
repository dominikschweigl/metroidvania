# Metroidvania CMake Setup (vcpkg + fmt)

This project uses CMake presets and vcpkg manifest mode. The repository itself does not require a specific compiler family, but local MinGW setup is recommended for this workspace.

## Project Structure

The project is organised in different folders serving the following purposes:

---

### Overview

```
assets/            # Images, audio, fonts
build/             # Build output (ignored in git)
data/              # This data serves as an inbetween step, between raw assets and usage in code.
  ├── animations/  # .anim/-files, which define the animations
  ├── entities/    # .entity-files, which are like blueprints for entity-objects
  └── maps/        # .json-files for defining worlds
dependencies/      # Third-party libraries (e.g. SFML)
docs/              # Documentation, because that is important
saves/             # Save gamestate (runtime-generated)
src/               # C++ source code (engine + gameplay)
  ├── core/        # Game loop, input, timing, window management
  ├── entities/    # Player, enemies, NPCs, and entity behavior
  ├── rendering/   # Sprites, animations, UI, HUD
  ├── utils/       # Helper functions, logging, file handling
  └── world/       # Maps, rooms, tiles, and environment logic
  main.cpp         # Entry point of the game.
tests/             # Unit and integration tests
  ├── data/        # Test-specific maps and configs
  ├── integration/
  └── unit/
```

---

### Notes

* **src/** contains all game logic and systems
* **assets/** for images, audio and other assets
* **data/** defines game objects (map, rooms, entity properties, etc.)
* **tests/** includes automated tests and their data also for CI
* **saves/** and **build/** are generated at runtime and not version-controlled

---

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


### EditorConfig Setup

This project uses an [.editorconfig](.editorconfig) file to enforce consistent coding styles across editors and platforms. It ensures settings like UTF-8 encoding, LF line endings, final newlines, and tab/space indentation are applied automatically.

- **Recommended Extension:** [EditorConfig for VS Code](https://marketplace.visualstudio.com/items?itemName=EditorConfig.EditorConfig)
- **Note:**
  - VS Code setting: `"files.insertFinalNewline": true` might be needed to enforce final newline for `.json` files


### Clang-Format Setup

Code formatting is managed by a [.clang-format](.clang-format) file, based on the LLVM style with project-specific tweaks (tab indentation, 4-space width, 120-column limit, etc.).

- **VS Code integration:**
  - Set C++ formatter to clang-format: `"C_Cpp.formatting": "clangFormat"`
  - Use the style from file: `"C_Cpp.clang_format_style": "file"`
  - Format on save: `"editor.formatOnSave": true`, `"editor.formatOnSaveMode": "file"`
  - Default formatter for C/C++: `"editor.defaultFormatter": "ms-vscode.cpptools"` in `[cpp]` and `[c]`

### IntelliSense Setup

VS Code C++ IntelliSense is configured for best compatibility with CMake:

- `"C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"` (CMake Tools integration)
- `"C_Cpp.intelliSenseEngine": "default"`
- `"C_Cpp.autocomplete": "default"`


### Note: Git LF/CRLF Warnings

If you see warnings `LF will be replaced by CRLF the next time Git touches it` during `git add`, Git line-ending conversion is enabled in your Git config.

Important: `.editorconfig` controls how your editor writes files, but Git warnings come from Git settings.

End-of-file LF for tracked text files is enforced by a `.gitattributes` file at repository root.

To disable conversion for this repository manually run following command.

```powershell
git config --local core.autocrlf false
```

### Terminal

Run with whatever preset names exist locally, for example:

```powershell
cmake --preset default-debug
cmake --build --preset default-debug
```

## Running Tests

### First-time setup

If `catch2` was recently added to `vcpkg.json`, reconfigure to install it:

```powershell
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
```

### Build and run

```powershell
# Build only the test executable
cmake --build ./build/release --target metroidvania_tests

# Run via CTest
ctest --test-dir ./build/release

# Or run directly
./build/release/metroidvania_tests.exe
```

### Notes on the test setup

- **`PLAYER_SOURCES`** — extracted in `CMakeLists.txt` so both the game and test executables share the same source list without duplicating it.
- **`WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"`** — required because state constructors load textures from relative paths like `"./assets/images/player/..."` at construction time. Without this, the working directory would be the build folder and texture loads would silently fail.
