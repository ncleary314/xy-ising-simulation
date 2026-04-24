#pragma once
#include <vector>

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

private:
    int latticeSize;
    double temperature;
    double couplingJ;
    double magneticFieldStrength;
    double magneticFieldAngle;

    std::vector<std::vector<double>> spinAngles;

    double computeLocalEnergy(int x, int y);
    int wrapIndex(int index);
};