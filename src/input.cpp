#include "input.h"

#include <cmath>

// Handles keyboard input and updates simulation parameters
void handleInput(SDL_Event& event, Simulation& sim) {

    if (event.type == SDL_KEYDOWN) {

        switch (event.key.keysym.sym) {

            // TEMPERATURE CONTROL
            case SDLK_UP: {
                sim.setTemperature(sim.getTemperature() + 0.1);
                break;
            }

            case SDLK_DOWN: {
                sim.setTemperature(sim.getTemperature() - 0.1);
                break;
            }

            // LATTICE SIZE CONTROL
            case SDLK_RIGHT: {
                sim.resize(sim.getSize() + 10);
                break;
            }

            case SDLK_LEFT: {
                sim.resize(sim.getSize() - 10);
                break;
            }

            // MAGNETIC FIELD
            case SDLK_w: {
                double newStrength = sim.getMagneticFieldStrength() + 0.1;
                sim.setMagneticField(newStrength, sim.getMagneticFieldAngle());
                break;
            }

            case SDLK_s: {
                double newStrength = sim.getMagneticFieldStrength() - 0.1;

                if (newStrength < 0.0) newStrength = 0.0;

                sim.setMagneticField(newStrength, sim.getMagneticFieldAngle());
                break;
            }

            case SDLK_a: {
                // Set field pointing left (π radians)
                sim.setMagneticField(sim.getMagneticFieldStrength(), M_PI);
                break;
            }

            case SDLK_d: {
                // Set field pointing right (0 radians)
                sim.setMagneticField(sim.getMagneticFieldStrength(), 0.0);
                break;
            }
        }
    }
}