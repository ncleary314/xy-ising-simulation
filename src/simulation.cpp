#include "simulation.h"
#include "constants.h"

#include <cmath>
#include <random>

// RANDOM NUMBER GENERATION
static std::mt19937 rng(std::random_device{}());

// Uniform distribution from 0 to 1
static std::uniform_real_distribution<double> uniformDist(0.0, 1.0);

// Generate a random angle between 0 and 2π
static double randomAngle() {
    return uniformDist(rng) * 2.0 * PI;
}

// CONSTRUCTOR
Simulation::Simulation(int size)
    : latticeSize(size),
      temperature(DEFAULT_TEMPERATURE),
      couplingJ(DEFAULT_J),
      magneticFieldStrength(DEFAULT_FIELD),
      magneticFieldAngle(0.0)
{
    // Initialize lattice with random spin directions
    spinAngles.resize(latticeSize, std::vector<double>(latticeSize));

    for (int x = 0; x < latticeSize; x++) {
        for (int y = 0; y < latticeSize; y++) {
            spinAngles[x][y] = randomAngle();
        }
    }
}

// PERIODIC BOUNDARY CONDITION
// Wraps indices so edges connect
int Simulation::wrapIndex(int index) {
    if (index < 0) return latticeSize - 1;
    if (index >= latticeSize) return 0;
    return index;
}

// LOCAL ENERGY
// Computes energy contribution of a single spin
double Simulation::computeLocalEnergy(int x, int y) {
    double theta = spinAngles[x][y];

    // Neighbor indices (periodic)
    int up    = wrapIndex(x - 1);
    int down  = wrapIndex(x + 1);
    int left  = wrapIndex(y - 1);
    int right = wrapIndex(y + 1);

    // Interaction term: alignment with neighbors
    double interactionEnergy =
        cos(theta - spinAngles[up][y]) +
        cos(theta - spinAngles[down][y]) +
        cos(theta - spinAngles[x][left]) +
        cos(theta - spinAngles[x][right]);

    interactionEnergy *= -couplingJ;

    // External magnetic field term
    double fieldEnergy = -magneticFieldStrength * cos(theta - magneticFieldAngle);

    return interactionEnergy + fieldEnergy;
}

// MONTE CARLO STEP
void Simulation::step() {
    // One sweep = N attempted updates
    int totalSites = latticeSize * latticeSize;

    for (int i = 0; i < totalSites; i++) {

        // Pick random lattice site
        int x = rng() % latticeSize;
        int y = rng() % latticeSize;

        double oldAngle = spinAngles[x][y];

        // Propose small random rotation
        double delta = (uniformDist(rng) - 0.5) * 0.5;
        double newAngle = oldAngle + delta;

        // Compute energy before change
        double oldEnergy = computeLocalEnergy(x, y);

        // Apply trial move
        spinAngles[x][y] = newAngle;

        // Compute energy after change
        double newEnergy = computeLocalEnergy(x, y);

        double deltaE = newEnergy - oldEnergy;

        // Metropolis acceptance rule
        if (deltaE > 0) {
            double acceptanceProbability = exp(-deltaE / temperature);

            if (uniformDist(rng) > acceptanceProbability) {
                // Reject and revert
                spinAngles[x][y] = oldAngle;
            }
        }
        // If ΔE ≤ 0 then automatically accepted
    }
}

// RESIZE LATTICE
void Simulation::resize(int newSize) {
    if (newSize < 10) newSize = 10;

    latticeSize = newSize;
    spinAngles.clear();

    spinAngles.resize(latticeSize, std::vector<double>(latticeSize));

    // Reinitialize spins randomly
    for (int x = 0; x < latticeSize; x++) {
        for (int y = 0; y < latticeSize; y++) {
            spinAngles[x][y] = randomAngle();
        }
    }
}

// SETTERS
void Simulation::setTemperature(double newTemp) {
    if (newTemp < 0.1) newTemp = 0.1;
    temperature = newTemp;
}

void Simulation::setMagneticField(double strength, double angle) {
    magneticFieldStrength = strength;
    magneticFieldAngle = angle;
}

// GETTERS
const std::vector<std::vector<double>>& Simulation::getSpins() const {
    return spinAngles;
}

double Simulation::getTemperature() const {
    return temperature;
}

double Simulation::getMagneticFieldStrength() const {
    return magneticFieldStrength;
}

double Simulation::getMagneticFieldAngle() const {
    return magneticFieldAngle;
}

int Simulation::getSize() const {
    return latticeSize;
}