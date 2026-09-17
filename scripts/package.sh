#!/usr/bin/env bash
#
# Build and package Metroidvania into a standalone, ready-to-upload archive.
#
# Usage:
#   scripts/package.sh [configure-preset] [build-preset]
#
# Defaults to the Linux presets. On success it writes:
#   dist/metroidvania-linux.zip   (Linux)
#   dist/metroidvania-macos.zip   (macOS, for local testing only)
#
# The archive is fully self-contained: the executable, its assets/ and data/
# folders, an empty writable saves/ folder, and any non-system shared libraries
# collected via ldd into lib/ (found at runtime through an $ORIGIN RPATH).
set -euo pipefail

CONFIGURE_PRESET="${1:-vcpkg-linux}"
BUILD_PRESET="${2:-linux}"

# Resolve repository root regardless of where the script is invoked from.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT_DIR}"

case "$(uname -s)" in
    Linux*)  OS_NAME="linux" ;;
    Darwin*) OS_NAME="macos" ;;
    *)       OS_NAME="unknown" ;;
esac

PACKAGE_NAME="metroidvania-${OS_NAME}"
BUILD_DIR="${ROOT_DIR}/build/${BUILD_PRESET}"
STAGE_DIR="${ROOT_DIR}/dist/${PACKAGE_NAME}"
ZIP_PATH="${ROOT_DIR}/dist/${PACKAGE_NAME}.zip"

echo "==> Configuring (${CONFIGURE_PRESET})"
cmake --preset "${CONFIGURE_PRESET}"

echo "==> Building (${BUILD_PRESET})"
cmake --build --preset "${BUILD_PRESET}" --target metroidvania

echo "==> Staging into ${STAGE_DIR}"
rm -rf "${STAGE_DIR}" "${ZIP_PATH}"
cmake --install "${BUILD_DIR}" --prefix "${STAGE_DIR}" --config Release

BINARY="${STAGE_DIR}/metroidvania"
if [[ ! -f "${BINARY}" ]]; then
    echo "ERROR: expected executable not found at ${BINARY}" >&2
    exit 1
fi

# --- Collect non-system shared libraries (Linux only) --------------------
# vcpkg's x64-linux triplet links its libraries statically, so this is usually
# a no-op. It is a safety net: if any dependency resolves to a shared object
# that is NOT part of the base OS / graphics stack, copy it into lib/ so the
# game runs on machines that lack it. System libraries (glibc, libGL, X11,
# udev, ...) are intentionally NOT bundled — they must come from the host.
if [[ "${OS_NAME}" == "linux" ]] && command -v ldd >/dev/null 2>&1; then
    echo "==> Collecting shared library dependencies (ldd)"
    SYSTEM_LIB_PATTERN='^(linux-vdso|ld-linux|libc|libm|libdl|libpthread|librt|libresolv|libutil|libstdc\+\+|libgcc_s|libGL|libGLX|libGLdispatch|libEGL|libOpenGL|libglapi|libX11|libXext|libXrandr|libXcursor|libXi|libXfixes|libXrender|libXau|libXdmcp|libxcb|libdrm|libudev|libdbus|libsystemd|libasound|libpulse|libgbm|libwayland)'
    mkdir -p "${STAGE_DIR}/lib"
    copied=0
    while read -r libpath; do
        [[ -f "${libpath}" ]] || continue
        libname="$(basename "${libpath}")"
        if [[ "${libname}" =~ ${SYSTEM_LIB_PATTERN} ]]; then
            continue
        fi
        cp -Lv "${libpath}" "${STAGE_DIR}/lib/"
        copied=$((copied + 1))
    done < <(ldd "${BINARY}" | awk '/=>/ {print $3} !/=>/ {print $1}' | grep '^/' || true)
    echo "    bundled ${copied} shared object(s) into lib/"
    # Drop the folder again if nothing needed bundling, to keep the tree tidy.
    rmdir "${STAGE_DIR}/lib" 2>/dev/null || true
fi

# --- macOS: add a double-clickable launcher -----------------------------
# A raw Unix executable launched from Finder starts with the working directory
# at "/", which breaks the game's relative ./assets and ./data lookups. Ship a
# .command launcher that cd's into its own folder first. vcpkg's *-osx triplets
# link SFML/fmt/OpenAL statically, and everything else the binary needs is a
# system framework, so no dependency collection is required on macOS.
if [[ "${OS_NAME}" == "macos" ]]; then
    echo "==> Adding macOS launcher"
    LAUNCHER="${STAGE_DIR}/Metroidvania.command"
    cat > "${LAUNCHER}" <<'LAUNCH'
#!/usr/bin/env bash
# Run the game from its own folder so it finds assets/ and data/.
cd "$(dirname "$0")"
exec ./metroidvania
LAUNCH
    chmod +x "${LAUNCHER}"
fi

# --- Ensure the executable is runnable after extraction (Itch.io) --------
chmod +x "${BINARY}"

# --- Compress -----------------------------------------------------------
echo "==> Creating ${ZIP_PATH}"
( cd "${ROOT_DIR}/dist" && zip -r -y -q "${PACKAGE_NAME}.zip" "${PACKAGE_NAME}" )

echo "==> Done: ${ZIP_PATH}"
ls -lh "${ZIP_PATH}"
