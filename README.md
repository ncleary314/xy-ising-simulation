# XY Ising Model Simulation (Monte Carlo, SDL2)

This project simulates the 2D XY (continuous Ising-like) model using a Monte Carlo Metropolis algorithm with SDL2 visualization.

# Features

- 2D XY spin lattice simulation
- Metropolis Monte Carlo updates
- Continuous spin angles (0 to 2π)
- Real-time SDL2 visualization
- Temperature control
- Lattice resizing
- Magnetic field control

# Dependencies

Required:
- CMake (>= 3.20)
- C++17 compiler
- SDL2
- SDL2_ttf

# macOS Setup

brew install cmake sdl2 sdl2_ttf

cmake --preset mac
cmake --build --preset mac

./build/xy_sim

# Windows Setup (vcpkg)

git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
bootstrap-vcpkg.bat

vcpkg install sdl2 sdl2-ttf

cmake --preset windows
cmake --build --preset windows

build\xy_sim.exe

# Controls

UP / DOWN = temperature
LEFT / RIGHT = lattice size
W / S = magnetic field strength
A / D = magnetic field direction

# Model

E = -J Σ cos(θᵢ - θⱼ) - h Σ cos(θᵢ - θ_h)

Metropolis Monte Carlo simulation of the XY model.

# Build Output

All build files are in /build and ignored by git.