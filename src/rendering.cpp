#include "rendering.h"
#include <cmath>
#include <SDL2/SDL_ttf.h>
#include <string>

static TTF_Font* font = nullptr;

// Converts spin direction into RGB color
SDL_Color angleToColor(double angle) {
    // Smooth color cycling using sine waves
    Uint8 r = static_cast<Uint8>(255 * (0.5 + 0.5 * sin(angle)));
    Uint8 g = static_cast<Uint8>(255 * (0.5 + 0.5 * sin(angle + 2.0)));
    Uint8 b = static_cast<Uint8>(255 * (0.5 + 0.5 * sin(angle + 4.0)));

    return {r, g, b, 255};
}

// RENDER LATTICE
void renderLattice(SDL_Renderer* renderer,
                   const std::vector<std::vector<double>>& spins,
                   int cellSize)
{
    int size = spins.size();

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {

            SDL_Color color = angleToColor(spins[x][y]);

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

            SDL_Rect cell;
            cell.x = x * cellSize;
            cell.y = y * cellSize;
            cell.w = cellSize;
            cell.h = cellSize;

            SDL_RenderFillRect(renderer, &cell);
        }
    }
}

// Initialize SDL_ttf and load font
void initText() {
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
    }

    font = TTF_OpenFont("assets/font.ttf", 28);

    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
    }
}

// Render text
void renderText(SDL_Renderer* renderer,
                const std::string& text,
                int x, int y)
{
    SDL_Color color = {255, 255, 255, 255};

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = surface->w;
    dest.h = surface->h;

    SDL_FreeSurface(surface);

    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_DestroyTexture(texture);
}