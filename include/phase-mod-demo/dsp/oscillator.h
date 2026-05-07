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

/** 
 * Increase the stored oscillator phase by the precalculated amount.
 * @par This function uses the floating point variant of Oscillator
 *
 * @param oscillator Pointer to oscillator struct
 * @see oscF16IncreasePhase() OscillatorF16 (Q16.16 fixed point) variant of this function
 */
void oscIncreasePhase(Oscillator* oscillator);

/**
 * Init Oscillator struct
 * @par This function uses the floating point variant of Oscillator
 *
 * @param osc Pointer to oscillator struct
 * @param table Pointer to the wavetable to use
 * @param tableLen Length of the wavetable
 * @param freq Frequency of the oscillator
 * @param modIndex Modulator index/depth when using this oscillator as a modulator
 * @param sampleRate The rate at which this oscillator will be sampled at. Typical is 44100 Hz. Used to calculate oscillator phase increment.
 */
void oscInit(Oscillator* osc, float *table, size_t tableLen, float freq, float modIndex, float sampleRate);

void oscUpdateFrequency(Oscillator* osc, float newFreq);

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
