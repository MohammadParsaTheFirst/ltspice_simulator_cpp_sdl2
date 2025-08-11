//
// Created by Parsa on 7/31/2025.
//

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <cmath>
const double PI = 3.141592;

// -------------------------------- Wave Forms Like Sinusoidal and DC and Pulse --------------------------------
class IWaveformStrategy {
public:
    virtual ~IWaveformStrategy() = default;
    virtual double getValue(double time) const = 0;
};

class DCWaveform : public IWaveformStrategy {
private:
    double dcValue;
public:
    DCWaveform(double value);
    double getValue(double time) const override;
    void setValue(double v) { dcValue = v; }
};

class SinusoidalWaveform : public IWaveformStrategy {
private:
    double offset, amplitude, frequency;
public:
    SinusoidalWaveform(double o, double a, double f);
    double getValue(double time) const override;
};
// -------------------------------- Wave Forms Like Sinusoidal and DC and Pulse --------------------------------

#endif //WAVEFORM_H