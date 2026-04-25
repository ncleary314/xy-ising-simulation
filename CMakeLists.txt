cmake_minimum_required(VERSION 3.20)

project(XYSimulation)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(xy_sim ${SOURCES})

# -----------------------------
# PROJECT HEADERS
# -----------------------------
target_include_directories(xy_sim PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# -----------------------------
# HOMEGROWN SDL FIX (IMPORTANT)
# -----------------------------
set(SDL2_ROOT "/opt/homebrew/opt/sdl2")
set(SDL2_TTF_ROOT "/opt/homebrew/opt/sdl2_ttf")

target_include_directories(xy_sim PRIVATE
    # SDL2 "namespaced" headers
    ${SDL2_ROOT}/include

    # ALSO expose flat-style compatibility for SDL_ttf
    ${SDL2_ROOT}/include/SDL2

    # SDL_ttf
    ${SDL2_TTF_ROOT}/include
)

target_link_directories(xy_sim PRIVATE
    ${SDL2_ROOT}/lib
    ${SDL2_TTF_ROOT}/lib
)

target_link_libraries(xy_sim
    SDL2
    SDL2_ttf
)