cmake_minimum_required(VERSION 3.10)

project(Metroidvania)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    src/scenes/menu_scene.cpp
    src/scenes/game_scene.cpp
    src/world/world.cpp
    src/entities/base/base_enemy.cpp
    src/entities/base/entity_physics.cpp
    src/entities/player/player.cpp
    src/entities/player/melee_attack.cpp
    src/entities/player/hat_ability.cpp
    src/entities/player/hat_projectile.cpp
    src/entities/player/states/idle_state.cpp
    src/entities/player/states/walking_state.cpp
    src/entities/player/states/running_state.cpp
    src/entities/player/states/pre_jump_state.cpp
    src/entities/player/states/ascending_state.cpp
    src/entities/player/states/peak_state.cpp
    src/entities/player/states/descending_state.cpp
    src/entities/player/states/landing_state.cpp
    src/entities/race_condition_slime/race_condition_slime.cpp
    src/entities/race_condition_slime/states/idle_state.cpp
    src/entities/race_condition_slime/states/chase_state.cpp
    src/entities/race_condition_slime/states/windup_state.cpp
    src/entities/race_condition_slime/states/attack_state.cpp
    src/entities/race_condition_slime/states/recover_state.cpp
)

add_executable(metroidvania src/main.cpp ${GAME_SOURCES})
target_link_libraries(metroidvania PRIVATE fmt::fmt SFML::Graphics SFML::Window SFML::System SFML::Audio)

add_executable(metroidvania_tests
    tests/unit/player_state_tests.cpp
    tests/unit/base_enemy_tests.cpp
    tests/unit/race_condition_slime_tests.cpp
    tests/unit/hat_projectile_tests.cpp
    tests/unit/input_manager_tests.cpp
    tests/unit/audio_manager_tests.cpp
    ${GAME_SOURCES})
target_include_directories(metroidvania_tests PRIVATE src)
target_link_libraries(metroidvania_tests PRIVATE Catch2::Catch2WithMain fmt::fmt SFML::Graphics SFML::Window SFML::System SFML::Audio)
catch_discover_tests(metroidvania_tests WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" DISCOVERY_MODE PRE_TEST)
