#include "simulation.h"
#include "constants.h"

#include <cmath>
#include <random>

// RANDOM NUMBER GENERATION
static std::mt19937 rng(std::random_device{}());

// Uniform distribution from 0 to 1
static std::uniform_real_distribution<double> uniformDist(0.0, 1.0);

static std::uniform_int_distribution<int> siteDist;

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
    proposalStepSize = 2.0;
    targetAcceptance = 0.5;
    tuningInterval = 5000; // number of attempted moves between adjustments
    acceptanceEMA = 0.5; // starting guess

    acceptedMoves = 0;
    attemptedMoves = 0;
    windowAccepted = 0;
    windowAttempted = 0;

    magnetization = 0.0;
    energyPerSpin = 0.0;
    vortexDensity = 0.0;
    helicityModulus = 0.0;

    helicityCosineTerm = 0.0;
    helicitySineTerm = 0.0;
    heatCapacity = 0.0;
    susceptibility = 0.0;

    magnetizationSum = 0.0;
    energySum = 0.0;
    magnetizationSquaredSum = 0.0;
    energySquaredSum = 0.0;
    helicityCosineSum = 0.0;
    helicitySineSquaredSum = 0.0;
    measurementCount = 0;
    sweepCount = 0;
    thermalizationSweeps = 5000;
    equilibrated = false;

    // initialize site distribution
    siteDist = std::uniform_int_distribution<int>(0, latticeSize - 1);

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

double Simulation::wrapAngleDifference(double dtheta) {

    while (dtheta <= -PI)
        dtheta += 2.0 * PI;

    while (dtheta > PI)
        dtheta -= 2.0 * PI;

    return dtheta;
}

int Simulation::countVortices() {

    int vortices = 0;

    vortexPositions.clear();

    for (int x = 0; x < latticeSize; x++) {
        for (int y = 0; y < latticeSize; y++) {

            int xp = wrapIndex(x + 1);
            int yp = wrapIndex(y + 1);

            double theta1 = spinAngles[x][y];
            double theta2 = spinAngles[xp][y];
            double theta3 = spinAngles[xp][yp];
            double theta4 = spinAngles[x][yp];

            double windingSum =
                wrapAngleDifference(theta2 - theta1) +
                wrapAngleDifference(theta3 - theta2) +
                wrapAngleDifference(theta4 - theta3) +
                wrapAngleDifference(theta1 - theta4);

            int windingNumber = static_cast<int>(std::round(windingSum / (2.0 * PI)));

            if (windingNumber != 0) {
                vortices += std::abs(windingNumber);

                vortexPositions.push_back({x, y});
            }
        }
    }

    return vortices;
}

void Simulation::computeObservables() {

    double sumCos = 0.0;
    double sumSin = 0.0;
    double totalEnergy = 0.0;
    double cosineSum = 0.0;     // Helicity Modulus
    double sineSum = 0.0;       // Helicity Modulus

    for (int x = 0; x < latticeSize; x++) {
        for (int y = 0; y < latticeSize; y++) {

            double theta = spinAngles[x][y];

            int xp = wrapIndex(x + 1);      // Helicity Modulus

            double deltaX = theta - spinAngles[xp][y];      // Helicity Modulus

            // Measures x-direction bond twists
            cosineSum += cos(deltaX);       // Helicity Modulus
            sineSum += sin(deltaX);     // Helicity Modulus

            sumCos += cos(theta);
            sumSin += sin(theta);

            totalEnergy += computeLocalEnergy(x, y);
        }
    }

    int N = latticeSize * latticeSize;

    // Magnetization magnitude
    magnetization = sqrt(sumCos * sumCos + sumSin * sumSin) / N;

    // Each bond counted twice, divide by 2
    energyPerSpin = totalEnergy / (2.0 * N);

    vortexDensity = static_cast<double>(countVortices()) / N;

    helicityCosineTerm = cosineSum / N;

    helicitySineTerm = (sineSum * sineSum) / N;

    // Υ = 1/N * ​⟨∑⟨ij⟩x​ ​cos(θi​−θj​)⟩ − 1/TN * ​⟨(∑⟨ij⟩x ​​sin(θi​−θj​))^2⟩
    // first term -> spin rigidity
    // second term -> thermal fluctuations
    // helicityModulus = (cosineSum / N) - (sineSum * sineSum) / (temperature * N);
}

// MONTE CARLO STEP
void Simulation::step() {
    // One sweep = N attempted updates
    int totalSites = latticeSize * latticeSize;

    for (int i = 0; i < totalSites; i++) {

        attemptedMoves++;
        windowAttempted++;

        // Pick random lattice site
        int x = siteDist(rng);
        int y = siteDist(rng);

        double oldAngle = spinAngles[x][y];

        // Propose small random rotation
        double delta = (uniformDist(rng) - 0.5) * proposalStepSize;
        double newAngle = oldAngle + delta;

        // Normalize to [0, 2π)
        newAngle = fmod(newAngle, 2.0 * PI);
        if (newAngle < 0) newAngle += 2.0 * PI;

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
                spinAngles[x][y] = oldAngle;
            } else {
                acceptedMoves++;
                windowAccepted++;
            }
        } else {
            acceptedMoves++;
            windowAccepted++;
        }
        // If ΔE ≤ 0 then automatically accepted
    }

    // Periodically adjust proposal step size
    // to maintain an efficient Metropolis acceptance rate.
    if (windowAttempted >= tuningInterval) {

        // Acceptance rate over recent tuning window
        double currentAcc = static_cast<double>(windowAccepted) / windowAttempted;

        // Smooth acceptance rate using exponential moving average
        // to reduce noisy fluctuations between windows.
        const double alpha = 0.05;
        acceptanceEMA = (1.0 - alpha) * acceptanceEMA + alpha * currentAcc;

        double accRate = acceptanceEMA;

        // Adapt proposal size:
        // - acceptance too high  -> proposals too small
        // - acceptance too low   -> proposals too large
        if (accRate > targetAcceptance) {
            proposalStepSize *= 1.1;
        } else {
            proposalStepSize *= 0.9;
        }

        // Keep proposal size within reasonable bounds
        if (proposalStepSize < 0.01)
            proposalStepSize = 0.01;

        if (proposalStepSize > 2.0 * PI)
            proposalStepSize = 2.0 * PI;

        // Reset tuning statistics for next window
        windowAccepted = 0;
        windowAttempted = 0;
    }

    computeObservables();

    if (equilibrated) {

        magnetizationSum += magnetization;
        energySum += energyPerSpin;

        magnetizationSquaredSum += magnetization * magnetization;

        energySquaredSum += energyPerSpin * energyPerSpin;

        helicityCosineSum += helicityCosineTerm;

        helicitySineSquaredSum += helicitySineTerm;

        measurementCount++;
    }

    if (measurementCount > 0) {

        double avgM = magnetizationSum / measurementCount;

        double avgM2 = magnetizationSquaredSum / measurementCount;

        double avgE = energySum / measurementCount;

        double avgE2 = energySquaredSum / measurementCount;

        double avgHelicityCosine = helicityCosineSum / measurementCount;

        double avgHelicitySineSquared = helicitySineSquaredSum / measurementCount;

        int N = latticeSize * latticeSize;

        susceptibility = (N / temperature) * (avgM2 - avgM * avgM);

        heatCapacity = (N / (temperature * temperature)) * (avgE2 - avgE * avgE);

        helicityModulus = avgHelicityCosine - (avgHelicitySineSquared / temperature);
    }

    sweepCount++;

    if (sweepCount >= thermalizationSweeps) {
        equilibrated = true;
    }
}

// RESIZE LATTICE
void Simulation::resize(int newSize) {
    if (newSize < 10) newSize = 10;

    latticeSize = newSize;
    siteDist = std::uniform_int_distribution<int>(0, latticeSize - 1);
    spinAngles.clear();

    spinAngles.resize(latticeSize, std::vector<double>(latticeSize));

    // Reinitialize spins randomly
    for (int x = 0; x < latticeSize; x++) {
        for (int y = 0; y < latticeSize; y++) {
            spinAngles[x][y] = randomAngle();
        }
    }

    sweepCount = 0;
    equilibrated = false;

    measurementCount = 0;
    magnetizationSum = 0.0;
    energySum = 0.0;
    magnetizationSquaredSum = 0.0;
    energySquaredSum = 0.0;
    helicityCosineSum = 0.0;
    helicitySineSquaredSum = 0.0;
}

// SETTERS
void Simulation::setTemperature(double newTemp) {
    if (newTemp < 0.1) newTemp = 0.1;
    temperature = newTemp;

    sweepCount = 0;
    equilibrated = false;

    measurementCount = 0;
    magnetizationSum = 0.0;
    energySum = 0.0;
    magnetizationSquaredSum = 0.0;
    energySquaredSum = 0.0;
    helicityCosineSum = 0.0;
    helicitySineSquaredSum = 0.0;
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

double Simulation::getAcceptanceRate() const {
    return acceptanceEMA;
}

double Simulation::getMagnetization() const {
    return magnetization;
}

double Simulation::getEnergyPerSpin() const {
    return energyPerSpin;
}

double Simulation::getVortexDensity() const {
    return vortexDensity;
}

const std::vector<std::pair<int,int>>&
Simulation::getVortexPositions() const {

    return vortexPositions;
}

double Simulation::getAverageMagnetization() const {
    if (measurementCount == 0) return 0.0;
    return magnetizationSum / measurementCount;
}

double Simulation::getAverageEnergy() const {
    if (measurementCount == 0) return 0.0;
    return energySum / measurementCount;
}

bool Simulation::isEquilibrated() const {
    return equilibrated;
}

double Simulation::getHelicityModulus() const {
    return helicityModulus;
}

double Simulation::getHeatCapacity() const {
    return heatCapacity;
}

double Simulation::getSusceptibility() const {
    return susceptibility;
}