#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

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
    bool autoSweepMode = false;
    bool dataSaved = false;

    int samplesCollected = 0;
    const int samplesPerTemperature = 200;
    const int sweepsPerSample = 10;     // Sweeps skipped between each sample
    int sweepsSinceLastSample = 0;

    SDL_Event event;

    double sweepTemperature = 0.2;
    const double maxTemperature = 2.0;
    const double temperatureStep = 0.1;
    double accumulatedEnergy = 0.0;
    double accumulatedMagnetization = 0.0;
    double accumulatedVortexDensity = 0.0;
    double accumulatedHelicity = 0.0;

    std::ofstream outputFile("temperature_scan.csv");
    outputFile << "T,E,M,V,Y,Cv,X\n";

    // MAIN LOOP
    while (running) {

        // HANDLE INPUT
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                running = false;
            }

            handleInput(event, simulation);

            if (event.type == SDL_KEYDOWN) {

                if (event.key.keysym.sym == SDLK_SPACE) {

                    autoSweepMode = true;

                    sweepTemperature = 0.2;

                    accumulatedEnergy = 0.0;
                    accumulatedMagnetization = 0.0;
                    accumulatedVortexDensity = 0.0;
                    accumulatedHelicity = 0.0;

                    samplesCollected = 0;
                    sweepsSinceLastSample = 0;

                    simulation.setTemperature(sweepTemperature);

                    dataSaved = false;
                }
            }
        }

        // PHYSICS UPDATE
        simulation.step();

        if (autoSweepMode && simulation.isEquilibrated()) {

            sweepsSinceLastSample++;

            if (sweepsSinceLastSample >= sweepsPerSample) {

                accumulatedEnergy += simulation.getEnergyPerSpin();

                accumulatedMagnetization += simulation.getMagnetization();

                accumulatedVortexDensity += simulation.getVortexDensity();

                accumulatedHelicity += simulation.getHelicityModulus();

                samplesCollected++;

                sweepsSinceLastSample = 0;
            }

            if (samplesCollected >= samplesPerTemperature) {

                double avgE = accumulatedEnergy / samplesCollected;

                double avgM = accumulatedMagnetization / samplesCollected;

                double avgV = accumulatedVortexDensity / samplesCollected;

                double avgY = accumulatedHelicity / samplesCollected;

                outputFile << simulation.getTemperature() << ","
                            << avgE << ","
                            << avgM << ","
                            << avgV << ","
                            << avgY << ","
                            << simulation.getHeatCapacity() << ","
                            << simulation.getSusceptibility() << "\n";

                sweepTemperature += temperatureStep;

                accumulatedEnergy = 0.0;
                accumulatedMagnetization = 0.0;
                accumulatedVortexDensity = 0.0;
                accumulatedHelicity = 0.0;

                samplesCollected = 0;
                sweepsSinceLastSample = 0;

                if (sweepTemperature > maxTemperature) {

                    autoSweepMode = false;

                    outputFile.close();

                    dataSaved = true;
                }
                else {

                    simulation.setTemperature(sweepTemperature);
                }
            }
        }

        // RENDERING
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        int cellSize = 800 / simulation.getSize();

        renderLattice(renderer, simulation.getSpins(), cellSize);

        renderVortices(renderer, simulation.getVortexPositions(), cellSize);

        // TEXT RENDERING
        std::stringstream line1;
        line1 << std::fixed << std::setprecision(2);

        line1 << "T = " << simulation.getTemperature()
              << " | Acc = " << simulation.getAcceptanceRate()
              << " | M = " << simulation.getMagnetization()
              << " | E = " << simulation.getEnergyPerSpin();

        renderText(renderer, line1.str(), 10, 10);

        std::stringstream line1b;
        line1b << std::fixed << std::setprecision(2);

        line1b << "Y = " << simulation.getHelicityModulus()
               << " | Cv = " << simulation.getHeatCapacity()
               << " | X = " << simulation.getSusceptibility();

        renderText(renderer, line1b.str(), 10, 45);

        std::stringstream line2;
        line2 << std::fixed << std::setprecision(2);

        line2 << "V = " << simulation.getVortexDensity()
              << " | Eq = " << (simulation.isEquilibrated() ? "YES" : "NO")
              << " | Auto = " << (autoSweepMode ? "ON" : "OFF")
              << " | Samples = " << samplesCollected
              << "/" << samplesPerTemperature;

        renderText(renderer, line2.str(), 10, 80);

        SDL_RenderPresent(renderer);
    }

    // CLEANUP
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
