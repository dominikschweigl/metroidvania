# Packaging for distribution (Itch.io)

This project ships as self-contained archives that a player can download,
extract, and run with **no system dependencies** — no SFML, no fmt, and no
Microsoft VC++ Redistributable:

| Platform | Archive                     |
| -------- | --------------------------- |
| Windows  | `metroidvania-windows.zip`  |
| Linux    | `metroidvania-linux.zip`    |
| macOS    | `metroidvania-macos.zip`    |

Each archive extracts to a single folder laid out exactly how the game expects
its files at runtime:

```
metroidvania-<os>/
  metroidvania[.exe]     # the game
  Metroidvania.command   # macOS only: double-clickable launcher
  *.dll                  # SFML / fmt / OpenAL DLLs (Windows dynamic builds only)
  assets/                # textures, audio, manifest.json  (loaded via ./assets/...)
  data/                  # Tiled maps + tilesets           (loaded via ./data/...)
  saves/                 # empty, writable — the game saves here at runtime
```

> The game resolves `assets/` and `data/` **relative to the working directory**,
> so these folders must stay next to the executable. The launcher on Itch.io runs
> the binary from its own folder, so this layout works out of the box.

---

## Building the archives

### Prerequisites

- [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set
- CMake ≥ 3.21, Ninja
- A toolchain for the target platform (MSVC or MinGW on Windows, GCC on Linux)

### Linux

```bash
./scripts/package.sh              # → dist/metroidvania-linux.zip
```

### Windows

```powershell
# From a "Developer PowerShell for VS" (so cl.exe is on PATH):
.\scripts\package.ps1             # → dist\metroidvania-windows.zip
```

### macOS

```bash
./scripts/package.sh vcpkg-macos macos    # → dist/metroidvania-macos.zip
```

Both scripts run configure → build → `cmake --install` into `dist/<name>/` →
`zip`. They are thin wrappers around the CMake install rules, so the archive
layout is identical no matter how you invoke them.

---

## How dependencies are made self-contained

### Windows

- **`MSVC_RUNTIME_LIBRARY = MultiThreaded`** links the **static C runtime** (`/MT`),
  so `msvcp140.dll` / `vcruntime140.dll` and the VC++ Redistributable are **not**
  needed for the game itself.
- With the default **`x64-windows`** (dynamic) triplet, SFML/fmt/OpenAL are DLLs.
  The install step bundles **every DLL the game needs, recursively**, next to the
  `.exe`: vcpkg's app-local deployment (`X_VCPKG_APPLOCAL_DEPS_INSTALL`) walks the
  full dependency chain with `dumpbin`, so not just `sfml-*`/`fmt` but also the
  DLLs *they* pull in — `freetype`, `openal32`, and the `ogg`/`vorbis`/`FLAC`
  codecs — are copied, and `package.ps1` mirrors any stragglers from the build
  tree. `InstallRequiredSystemLibraries` adds the CRT (`msvcp140`/`vcruntime140`).
  (`$<TARGET_RUNTIME_DLLS>` alone only sees the exe's *direct* deps and misses the
  second-level ones — that is why the app-local step is needed.)
- For a **single dependency-free executable**, build fully static instead:

  ```powershell
  .\scripts\package.ps1 vcpkg-windows-msvc-static windows-msvc-static   # /MT + static SFML
  # or MinGW:
  .\scripts\package.ps1 vcpkg-windows-mingw       windows-mingw         # -static everything
  ```

### Linux

- The vcpkg **`x64-linux`** triplet builds SFML, fmt and OpenAL as **static**
  libraries, so they are baked into the binary.
- `-static-libgcc -static-libstdc++` fold the GCC/C++ runtimes in too.
- An **`$ORIGIN` RPATH** makes the binary look for any remaining shared libraries
  in its own folder and `./lib`. `scripts/package.sh` uses `ldd` to collect any
  **non-system** shared object into `lib/` as a safety net.
- Base-OS libraries (glibc, libGL, X11, libudev, …) are intentionally **not**
  bundled — they must come from the host, as on every desktop Linux install.
- The script sets the executable bit (`chmod +x`) before zipping so players do
  not hit "Permission denied" after extracting on Itch.io.

### macOS

- The vcpkg `arm64-osx` / `x64-osx` triplets build SFML, fmt and OpenAL as
  **static** libraries, and the remaining dependencies are system frameworks
  (Cocoa, OpenGL, IOKit) present on every Mac — so nothing needs bundling.
- The archive includes a `Metroidvania.command` launcher, because a raw
  executable opened from Finder starts with its working directory at `/`, which
  would break the relative `./assets` / `./data` lookups. The launcher `cd`s
  into its own folder first. Both it and the binary are marked executable.
- **Gatekeeper:** the app is unsigned, so on first launch macOS shows
  "unidentified developer". Players right-click → **Open** (once), or run
  `xattr -dr com.apple.quarantine <extracted folder>`. Removing this friction
  entirely requires an Apple Developer ID plus code-signing and notarization.

---

## Continuous delivery

`.github/workflows/release.yml` builds and uploads all three archives on each
target OS.

**A GitHub Release is only cut from a version tag that lives on `main`.** The
workflow's `guard` job checks that the tagged commit is reachable from
`origin/main` and fails otherwise, so tagging a feature branch cannot publish a
release. The intended flow is therefore: merge to `main` first, then tag `main`:

```bash
# after the packaging PR is merged into main
git checkout main
git pull
git tag v1.0.0
git push origin v1.0.0
```

On such a tag push the archives are attached to a GitHub Release, ready to
download and upload to Itch.io.

You can also trigger the workflow manually from the **Actions** tab
(`workflow_dispatch`) for an artifact-only build — this uploads the zips as run
artifacts but does not create a Release.

---

## Fonts (note)

The UI font is loaded from the operating system (Arial on Windows, Liberation/
DejaVu Sans on Linux, Arial/Helvetica on macOS) rather than bundled. These are
present on all standard Windows, desktop-Linux and macOS installs, so no action
is needed for typical players.
