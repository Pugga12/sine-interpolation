#include <stdint.h>
#include <stdlib.h>
#include "fix16.h"
#include "oscillator.h"

void oscIncreasePhase(Oscillator* oscillator) {
    oscillator->phase += oscillator->phaseIncrement;
    if (oscillator->phase >= oscillator->tableLen) oscillator->phase -= oscillator->tableLen;
}

void oscInit(Oscillator* osc, int16_t *table, size_t tableLen, float freq, float mod_index, float sample_rate) {
    osc->table = table;
    osc->tableLen = tableLen;
    osc->phase = 0.0f;
    osc->oscillatorFrequency = freq;
    osc->modIndex = mod_index;
    osc->phaseIncrement = (tableLen * freq) / sample_rate;
    osc->sampleRate = sample_rate;
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