<#
.SYNOPSIS
    Build and package Metroidvania into a standalone, ready-to-upload Windows archive.

.DESCRIPTION
    Configures, builds and installs the game into a staging directory, then zips it
    into dist\metroidvania-windows.zip. The archive is self-contained:

        metroidvania.exe
        *.dll            (SFML / fmt / OpenAL, only for dynamic builds)
        assets\          (textures, audio, manifest)
        data\            (Tiled maps + tilesets)
        saves\           (empty, writable at runtime)

    Use the default (vcpkg-windows-msvc) triplet to ship SFML DLLs alongside the exe
    with a static CRT (/MT, so no VC++ Redistributable is required), or pass the
    static presets for a single dependency-free executable:

        scripts\package.ps1 vcpkg-windows-msvc-static windows-msvc-static
        scripts\package.ps1 vcpkg-windows-mingw       windows-mingw

.PARAMETER ConfigurePreset
    CMake configure preset to use. Default: vcpkg-windows-msvc.

.PARAMETER BuildPreset
    CMake build preset to use. Default: windows-msvc.
#>
param(
    [string]$ConfigurePreset = "vcpkg-windows-msvc",
    [string]$BuildPreset     = "windows-msvc"
)

$ErrorActionPreference = "Stop"

$RootDir     = Split-Path -Parent $PSScriptRoot
$PackageName = "metroidvania-windows"
$BuildDir    = Join-Path $RootDir "build\$BuildPreset"
$StageDir    = Join-Path $RootDir "dist\$PackageName"
$ZipPath     = Join-Path $RootDir "dist\$PackageName.zip"

Set-Location $RootDir

Write-Host "==> Configuring ($ConfigurePreset)"
cmake --preset $ConfigurePreset
if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }

Write-Host "==> Building ($BuildPreset)"
cmake --build --preset $BuildPreset --target metroidvania
if ($LASTEXITCODE -ne 0) { throw "cmake build failed" }

Write-Host "==> Staging into $StageDir"
if (Test-Path $StageDir) { Remove-Item -Recurse -Force $StageDir }
if (Test-Path $ZipPath)  { Remove-Item -Force $ZipPath }
cmake --install $BuildDir --prefix $StageDir --config Release
if ($LASTEXITCODE -ne 0) { throw "cmake install failed" }

$Executable = Join-Path $StageDir "metroidvania.exe"
if (-not (Test-Path $Executable)) {
    throw "Expected executable not found at $Executable"
}

# Safety net for dynamic builds: vcpkg's build-time app-local deployment copies
# every transitive DLL (sfml-*, fmt, freetype, ogg/vorbis/FLAC, openal32, CRT)
# next to the built exe. Mirror any that aren't already staged, so the archive
# is never missing a dependency even if an install-time copy was skipped.
$stagedDlls = @{}
Get-ChildItem -Path $StageDir -Filter *.dll -File -ErrorAction SilentlyContinue |
    ForEach-Object { $stagedDlls[$_.Name] = $true }
$missing = 0
Get-ChildItem -Path $BuildDir -Filter *.dll -File -ErrorAction SilentlyContinue | ForEach-Object {
    if (-not $stagedDlls.ContainsKey($_.Name)) {
        Copy-Item $_.FullName -Destination $StageDir -Force
        Write-Host "    + bundled missing DLL: $($_.Name)"
        $missing++
    }
}
Write-Host "==> DLL check: copied $missing DLL(s) that install had missed"

Write-Host "==> Creating $ZipPath"
Compress-Archive -Path $StageDir -DestinationPath $ZipPath -Force

Write-Host "==> Done: $ZipPath"
Get-Item $ZipPath | Format-List Name, Length, FullName
