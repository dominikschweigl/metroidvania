cmake_minimum_required(VERSION 3.10)

project(Metroidvania)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(SFML CONFIG REQUIRED COMPONENTS Graphics Window System)

add_executable(metroidvania src/main.cpp src/world/world.cpp)

target_link_libraries(metroidvania PRIVATE fmt::fmt SFML::Graphics SFML::Window SFML::System)
