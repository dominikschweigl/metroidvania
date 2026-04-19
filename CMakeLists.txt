cmake_minimum_required(VERSION 3.10)

project(Metroidvania)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(SFML CONFIG REQUIRED COMPONENTS Graphics Window System)

add_executable(metroidvania
    src/main.cpp
    src/world/world.cpp
    src/entities/player/player.cpp
    src/entities/player/states/idle_state.cpp
    src/entities/player/states/walking_state.cpp
    src/entities/player/states/running_state.cpp
    src/entities/player/states/pre_jump_state.cpp
    src/entities/player/states/ascending_state.cpp
    src/entities/player/states/peak_state.cpp
    src/entities/player/states/descending_state.cpp
    src/entities/player/states/landing_state.cpp
)

target_link_libraries(metroidvania PRIVATE fmt::fmt SFML::Graphics SFML::Window SFML::System)
