#include "WaveForm.h"
#include <cereal/types/polymorphic.hpp>
#include <cereal/details/static_object.hpp>

// -------------------------------- Constructor impementation --------------------------------
DCWaveform::DCWaveform(double value) : dcValue(value) {}
SinusoidalWaveform::SinusoidalWaveform(double o, double a, double f) : offset(o), amplitude(a), frequency(f) {}


// -------------------------------- Get value --------------------------------
double DCWaveform::getValue(double time) const {return dcValue;}
double SinusoidalWaveform::getValue(double time) const {return offset + amplitude * sin(2 * PI * frequency * time);}

CEREAL_DYNAMIC_INIT(waveforms) {
    CEREAL_REGISTER_TYPE(DCWaveform)
    CEREAL_REGISTER_TYPE(SinusoidalWaveform)
    CEREAL_REGISTER_POLYMORPHIC_RELATION(IWaveformStrategy, DCWaveform)
    CEREAL_REGISTER_POLYMORPHIC_RELATION(IWaveformStrategy, SinusoidalWaveform)
}