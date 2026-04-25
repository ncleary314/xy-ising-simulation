cmake_minimum_required(VERSION 3.20)

project(XYSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(xy_sim ${SOURCES})

# -----------------------------
# PROJECT HEADERS
# -----------------------------
target_include_directories(xy_sim PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# -----------------------------
# SDL2 (PORTABLE WAY)
# -----------------------------

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

target_link_libraries(xy_sim
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)