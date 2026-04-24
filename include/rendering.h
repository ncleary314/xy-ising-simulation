#pragma once
#include <SDL2/SDL.h>
#include <vector>

void renderLattice(SDL_Renderer* renderer,
                   const std::vector<std::vector<double>>& spins,
                   int cellSize);

void initText();
void renderText(SDL_Renderer* renderer,
                const std::string& text,
                int x, int y);