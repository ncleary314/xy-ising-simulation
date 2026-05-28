#pragma once
#include <vector>
#include <utility>

class Simulation {
public:
    Simulation(int size);

    void step();  // One Monte Carlo step
    void resize(int newSize);

    void setTemperature(double newTemp);
    void setMagneticField(double strength, double angle);

    const std::vector<std::vector<double>>& getSpins() const;

    double getTemperature() const;
    double getMagneticFieldStrength() const;
    double getMagneticFieldAngle() const;
    int getSize() const;
    double getAcceptanceRate() const;
    double getMagnetization() const;
    double getEnergyPerSpin() const;
    double getVortexDensity() const;
    double getHelicityModulus() const;
    double getHeatCapacity() const;
    double getSusceptibility() const;
    const std::vector<std::pair<int,int>>& getVortexPositions() const;

    double getAverageMagnetization() const;
    double getAverageEnergy() const;

    bool isEquilibrated() const;

private:
    int latticeSize;
    double temperature;
    double couplingJ;
    double magneticFieldStrength;
    double magneticFieldAngle;
    double proposalStepSize;
    int acceptedMoves;
    int attemptedMoves;

    // Windowed counters (for real-time accuracy)
    int windowAccepted;
    int windowAttempted;

    double targetAcceptance;
    int tuningInterval;
    double acceptanceEMA;

    // Instantaneous observables
    double magnetization;
    double energyPerSpin;
    double vortexDensity;
    double helicityModulus;
    double helicityCosineTerm;
    double helicitySineTerm;
    double heatCapacity;
    double susceptibility;
    std::vector<std::pair<int,int>> vortexPositions;

    // Running averages
    double magnetizationSum;
    double energySum;
    double magnetizationSquaredSum;
    double energySquaredSum;
    double helicityCosineSum;
    double helicitySineSquaredSum;
    int measurementCount;

    int sweepCount;
    int thermalizationSweeps;
    bool equilibrated;

    std::vector<std::vector<double>> spinAngles;

    double computeLocalEnergy(int x, int y);
    double wrapAngleDifference(double dtheta);
    int countVortices();
    int wrapIndex(int index);

    void computeObservables();
};