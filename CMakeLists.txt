# 3.21 is required for the $<TARGET_RUNTIME_DLLS:...> generator expression used by
# the packaging rules below to bundle SFML/fmt/OpenAL DLLs on Windows.
cmake_minimum_required(VERSION 3.21)

project(Metroidvania VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Honour MSVC_RUNTIME_LIBRARY (static-CRT selection). NEW by default on 3.21,
# set explicitly so intent is documented and robust against policy changes.
cmake_policy(SET CMP0091 NEW)

# Match the C runtime to the vcpkg triplet's CRT linkage, project-wide (game AND
# tests), set before any target is created. The x64-windows-static triplet links
# everything (SFML, Catch2, fmt) against the STATIC CRT, so our objects must use
# /MT too or the linker raises LNK2038 (MT vs MD mismatch). The default dynamic
# triplet keeps CMake's default (/MD), matching the DLL/import-lib dependencies.
# A static build therefore needs no VC++ Redistributable; the dynamic build ships
# the CRT DLLs instead (see InstallRequiredSystemLibraries below).
if (MSVC AND VCPKG_TARGET_TRIPLET MATCHES "static")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(SFML CONFIG REQUIRED COMPONENTS Graphics Window System Audio)
find_package(Catch2 CONFIG REQUIRED)
include(CTest)
include(Catch)

set(GAME_SOURCES
    src/core/asset_manager.cpp
    src/core/audio_manager.cpp
    src/core/input_manager.cpp
    src/core/scene_stack.cpp
    src/ui/audio_settings_list.cpp
    src/ui/binding_list.cpp
    src/ui/button.cpp
    src/ui/confirm_dialog.cpp
    src/ui/content_with_back.cpp
    src/ui/slider.cpp
    src/ui/vertical_list.cpp
    src/ui/panel.cpp
    src/ui/health_bar.cpp
    src/ui/hotbar_hud.cpp
    src/ui/interaction_indicator.cpp
    src/ui/minimap.cpp
    src/scenes/menu_scene.cpp
    src/scenes/bluescreen_scene.cpp
    src/scenes/victory_scene.cpp
    src/scenes/dialogue_scene.cpp
    src/scenes/story_snippets.cpp
    src/scenes/game_scene.cpp
    src/scenes/inventory_scene.cpp
    src/world/Room.cpp
    src/world/world.cpp
    src/combat/combat_system.cpp
    src/entities/base_entity.cpp
    src/entities/entity_physics.cpp
    src/entities/enemies/base_enemy.cpp
    src/entities/enemies/race_condition_slime/race_condition_slime.cpp
    src/entities/enemies/race_condition_slime/states/idle_state.cpp
    src/entities/enemies/race_condition_slime/states/chase_state.cpp
    src/entities/enemies/race_condition_slime/states/windup_state.cpp
    src/entities/enemies/race_condition_slime/states/attack_state.cpp
    src/entities/enemies/race_condition_slime/states/recover_state.cpp
    src/entities/enemies/resistor_bug/resistor_bug.cpp
    src/entities/enemies/resistor_bug/states/idle_state.cpp
    src/entities/enemies/resistor_bug/states/chase_state.cpp
    src/entities/enemies/resistor_bug/states/jump_attack_state.cpp
    src/entities/enemies/resistor_bug/states/recover_state.cpp
    src/entities/enemies/recursion_golem/recursion_golem.cpp
    src/entities/enemies/recursion_golem/states/idle_state.cpp
    src/entities/enemies/recursion_golem/states/chase_state.cpp
    src/entities/enemies/recursion_golem/states/windup_state.cpp
    src/entities/enemies/recursion_golem/states/attack_state.cpp
    src/entities/enemies/recursion_golem/states/explode_state.cpp
    src/effects/effect.cpp
    src/items/item_registry.cpp
    src/items/healing_potion_item.cpp
    src/items/hat_item.cpp
    src/items/chewing_gum_item.cpp
    src/items/jump_potion_item.cpp
    src/items/speed_potion_item.cpp
    src/items/resistance_potion_item.cpp
    src/items/damage_potion_item.cpp
    src/items/usb_key_item.cpp
    src/items/backup_disk_item.cpp
    src/items/world_item.cpp
    src/entities/player/inventory.cpp
    src/entities/player/player.cpp
    src/entities/player/abilities/melee_attack.cpp
    src/entities/player/abilities/hat_ability.cpp
    src/entities/player/abilities/hat_projectile.cpp
    src/entities/player/states/idle_state.cpp
    src/entities/player/states/walking_state.cpp
    src/entities/player/states/running_state.cpp
    src/entities/player/states/pre_jump_state.cpp
    src/entities/player/states/ascending_state.cpp
    src/entities/player/states/peak_state.cpp
    src/entities/player/states/descending_state.cpp
    src/entities/player/states/landing_state.cpp
    src/entities/player/states/wall_slide_state.cpp
    src/entities/projectiles/electric_ball.cpp
    src/entities/enemies/capacitor/capacitor.cpp
    src/entities/enemies/capacitor/states/hover_state.cpp
    src/entities/enemies/capacitor/states/shoot_state.cpp
    src/entities/enemies/capacitor/states/flee_state.cpp
    src/entities/enemies/capacitor/states/swoop_state.cpp
    src/entities/enemies/bosses/transistor_boss/transistor_boss.cpp
    src/entities/enemies/bosses/transistor_boss/transistor_boss_renderer.cpp
    src/entities/enemies/bosses/transistor_boss/states/roaming_state.cpp
    src/entities/enemies/bosses/transistor_boss/states/shoot_attack_state.cpp
    src/entities/enemies/bosses/transistor_boss/states/charge_attack_windup_state.cpp
    src/entities/enemies/bosses/transistor_boss/states/charge_attack_state.cpp
    src/entities/enemies/bosses/transistor_boss/states/recover_state.cpp
    src/entities/enemies/bosses/transistor_boss/states/stage2_recover_state.cpp
    src/entities/enemies/bosses/transistor_boss/states/summon_state.cpp
    src/entities/enemies/bosses/transistor_boss/states/death_state.cpp
    src/entities/enemies/bosses/segfault_boss/segfault_boss.cpp
    src/entities/enemies/bosses/segfault_boss/segfault_boss_renderer.cpp
    src/entities/enemies/bosses/segfault_boss/states/roaming_state.cpp
    src/entities/enemies/bosses/segfault_boss/states/null_spear_attack_state.cpp
    src/entities/enemies/bosses/segfault_boss/states/recover_state.cpp
    src/entities/enemies/bosses/segfault_boss/states/stage2_transition_state.cpp
    src/entities/enemies/bosses/segfault_boss/states/summon_state.cpp
    src/entities/enemies/bosses/segfault_boss/states/stage3_transition_state.cpp
    src/entities/enemies/bosses/segfault_boss/states/fork_state.cpp
    src/entities/enemies/bosses/segfault_boss/states/death_state.cpp
)

add_executable(metroidvania src/main.cpp ${GAME_SOURCES})
target_include_directories(metroidvania PRIVATE src external/tileson)
target_link_libraries(metroidvania PRIVATE fmt::fmt SFML::Graphics SFML::Window SFML::System SFML::Audio)

# ---------------------------------------------------------------------------
# Self-contained runtime linking (so end users need no system dependencies)
# The MSVC static CRT is configured project-wide above (CMAKE_MSVC_RUNTIME_LIBRARY).
# ---------------------------------------------------------------------------
if (MINGW)
    # MinGW/GCC on Windows: fold libstdc++, libgcc and libwinpthread into the exe
    # so no MinGW runtime DLLs need to ship alongside it.
    target_link_options(metroidvania PRIVATE -static -static-libgcc -static-libstdc++)
elseif (UNIX AND NOT APPLE)
    # Linux: statically link the GCC/C++ runtimes, and make the binary look for
    # any remaining shared libraries next to itself (./ and ./lib) via $ORIGIN,
    # so a bundled lib/ folder is found without setting LD_LIBRARY_PATH.
    target_link_options(metroidvania PRIVATE -static-libgcc -static-libstdc++)
    set_target_properties(metroidvania PROPERTIES
        INSTALL_RPATH "$ORIGIN;$ORIGIN/lib"
        BUILD_WITH_INSTALL_RPATH ON)
endif()

add_executable(metroidvania_tests
    tests/unit/player_state_tests.cpp
    tests/unit/base_entity_tests.cpp
    tests/unit/base_enemy_tests.cpp
    tests/unit/race_condition_slime_tests.cpp
    tests/unit/resistor_bug_tests.cpp
    tests/unit/recursion_golem_tests.cpp
    tests/unit/capacitor_tests.cpp
    tests/unit/transistor_boss_tests.cpp
    tests/unit/segfault_boss_tests.cpp
    tests/unit/hat_projectile_tests.cpp
    tests/unit/input_manager_tests.cpp
    tests/unit/audio_manager_tests.cpp
    tests/unit/health_tests.cpp
    tests/unit/hitbox_tests.cpp
    tests/unit/combat_system_tests.cpp
    tests/unit/player_combat_tests.cpp
    tests/unit/melee_attack_tests.cpp
    tests/unit/inventory_tests.cpp
    tests/unit/effect_tests.cpp
    tests/unit/debug_features_tests.cpp
    tests/unit/interaction_indicator_tests.cpp
    tests/unit/entity_physics_tests.cpp
    tests/unit/world_item_tests.cpp
    tests/unit/dialogue_scene_tests.cpp
    ${GAME_SOURCES})
target_include_directories(metroidvania_tests PRIVATE src)
target_include_directories(metroidvania_tests PRIVATE src external/tileson)
target_link_libraries(metroidvania_tests PRIVATE Catch2::Catch2WithMain fmt::fmt SFML::Graphics SFML::Window SFML::System SFML::Audio)
catch_discover_tests(metroidvania_tests WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" DISCOVERY_MODE PRE_TEST)

if (NOT CMAKE_CXX_CLANG_TIDY)
    if(MSVC)
        string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        target_compile_options(metroidvania PRIVATE /W4 /WX /permissive- /utf-8)
        target_compile_options(metroidvania_tests PRIVATE /W4 /WX /permissive- /utf-8)
    else()
        target_compile_options(metroidvania PRIVATE -Wall -Wextra)
        target_compile_options(metroidvania_tests PRIVATE -Wall -Wextra)
    endif()
endif()

# ===========================================================================
# Installation & packaging (produces the Itch.io distributable layout)
#
#   <prefix>/
#     metroidvania[.exe]      <- the game
#     *.dll                   <- SFML/fmt/OpenAL DLLs (Windows dynamic builds)
#     assets/                 <- textures, audio, manifest (loaded via ./assets)
#     data/                   <- Tiled maps + tilesets (loaded via ./data)
#     saves/                  <- writable, empty; the game saves here at runtime
#
# `cmake --install <build> --prefix <stage>` lays this out; the scripts in
# scripts/ then zip <stage> into metroidvania-<os>.zip. See docs/PACKAGING.md.
# ===========================================================================

# The game itself, at the archive root.
#
# On Windows dynamic builds, tell the vcpkg toolchain to also copy every DLL the
# executable transitively needs into the install tree. Unlike
# $<TARGET_RUNTIME_DLLS> (which only sees the exe's DIRECT link dependencies —
# sfml-*, fmt), this walks the full dependency chain with dumpbin, so the DLLs
# that the SFML DLLs themselves pull in (freetype, ogg/vorbis/FLAC, openal32)
# are bundled too. No effect on static builds or non-vcpkg configs.
if (WIN32)
    set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
endif()

install(TARGETS metroidvania RUNTIME DESTINATION ".")

# Runtime data the game opens through relative paths — must sit beside the exe.
install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets" DESTINATION ".")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/data"   DESTINATION ".")

# World::saveWorldData writes to "saves/<world>.json" but does not create the
# folder, so ship an empty one; otherwise saving fails on a fresh extract.
install(DIRECTORY DESTINATION "saves")

# $<TARGET_RUNTIME_DLLS> covers the exe's direct dependencies (sfml-*, fmt) as a
# fallback; the vcpkg app-local step above is what pulls in the transitive DLLs.
# For a fully static build (x64-mingw-static / x64-windows-static) both are empty.
if (WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:metroidvania> DESTINATION ".")

    # Only needed for DYNAMIC builds: the /MD-built SFML DLLs require the shared
    # CRT (msvcp140.dll / vcruntime140.dll), so ship it — then no VC++
    # Redistributable install is required. A static triplet (x64-windows-static)
    # bakes everything into the exe, so skip it and keep the archive DLL-free.
    if (MSVC AND NOT VCPKG_TARGET_TRIPLET MATCHES "static")
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP OFF)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
        include(InstallRequiredSystemLibraries)
    endif()
endif()
