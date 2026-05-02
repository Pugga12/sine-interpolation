#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "fix16.h"

typedef struct
{
    float* table;
    size_t tableLen;
    float phase;
    float phaseIncrement;
    float oscillatorFrequency;
    float modIndex;
    float sampleRate;
} Oscillator;

void oscIncreasePhase(Oscillator* oscillator);

void oscInit(Oscillator* osc, float *table, size_t tableLen, float freq, float mod_index, float sample_rate);

typedef struct
{
    fix16_t* table;
    int32_t tableLen;
    float phase;
    float phaseIncrement;
    float oscillatorFrequency;
    float modIndex;
    float sampleRate;
} OscillatorF16;

void oscF16IncreasePhase(OscillatorF16* oscillator);

void oscInitF16(OscillatorF16* osc, fix16_t *table, int32_t tableLen, float freq, float mod_index, float sample_rate);