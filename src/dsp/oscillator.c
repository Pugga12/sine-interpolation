#include <stdint.h>
#include <stdlib.h>
#include "fix16.h"
#include "dsp/oscillator.h"

#include "dsp/noise.h"

void oscIncreasePhase(Oscillator* oscillator) {
    // const float jitter = 1.0f + (gaussianRandom() * 0.001f);
    oscillator->phase += oscillator->phaseIncrement;
    if (oscillator->phase >= oscillator->tableLen) oscillator->phase -= oscillator->tableLen;
}

void oscInit(Oscillator* osc, float *table, size_t tableLen, float freq, float modIndex, float sampleRate) {
    osc->table = table;
    osc->tableLen = tableLen;
    osc->phase = 0.0f;
    osc->oscillatorFrequency = freq;
    osc->modIndex = modIndex;
    osc->phaseIncrement = ((float)tableLen * freq) / sampleRate;
    osc->sampleRate = sampleRate;
}

void oscUpdateFrequency(Oscillator *osc, float newFreq) {
    osc->phase = 0.0f;
    osc->oscillatorFrequency = newFreq;
    osc->phaseIncrement = ((float)osc->tableLen * newFreq) / osc->sampleRate;
}

void oscF16IncreasePhase(OscillatorF16* oscillator) {
    oscillator->phase += oscillator->phaseIncrement;
    if (oscillator->phase >= oscillator->tableLen) {
        oscillator->phase -= oscillator->tableLen;
    } 
}

void oscInitF16(OscillatorF16* osc, fix16_t *table, int32_t tableLen, float freq, float mod_index, float sample_rate) {
    osc->table = table;
    osc->tableLen = tableLen;
    osc->phase = 0.0f;
    osc->oscillatorFrequency = freq;
    osc->modIndex = mod_index;
    osc->phaseIncrement = (tableLen * freq) / sample_rate;
    osc->sampleRate = sample_rate;
}
