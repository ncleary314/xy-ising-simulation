#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "simulation.h"
#include "rendering.h"
#include "input.h"
#include "constants.h"

int main() {

    // SDL INITIALIZATION
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize\n";
        return -1;
    }

    initText();

    SDL_Window* window = SDL_CreateWindow(
        "XY Model Simulation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        800,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    // SIMULATION
    Simulation simulation(DEFAULT_LATTICE_SIZE);

    bool running = true;
    SDL_Event event;

    // MAIN LOOP
    while (running) {

        // HANDLE INPUT
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                running = false;
            }

            handleInput(event, simulation);
        }

        // PHYSICS UPDATE
        simulation.step();

        // RENDERING
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        int cellSize = 800 / simulation.getSize();

        renderLattice(renderer, simulation.getSpins(), cellSize);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "T = " << simulation.getTemperature();

        renderText(renderer, ss.str(), 10, 10);

        SDL_RenderPresent(renderer);
    }

    // CLEANUP
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
