#include <stdint.h>
#include <stdlib.h>
typedef struct
{
    int16_t* table;
    size_t tableLen;
    float phase;
    float phaseIncrement;
    float oscillatorFrequency;
    float modIndex;
    float sampleRate;
} Oscillator;

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