//
// Created by parsa on 7/31/2025.
//

#include "WaveForm.h"

// -------------------------------- Constructor impementation --------------------------------
DCWaveform::DCWaveform(double value) : dcValue(value) {}

SinusoidalWaveform::SinusoidalWaveform(double o, double a, double f) : offset(o), amplitude(a), frequency(f) {}
// -------------------------------- Constructor impementation --------------------------------


// -------------------------------- Get value --------------------------------
double DCWaveform::getValue(double time) const {
    return dcValue;
}

double SinusoidalWaveform::getValue(double time) const {
    return offset + amplitude * sin(2 * PI * frequency * time);
}
// -------------------------------- Get value --------------------------------