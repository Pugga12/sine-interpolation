//
// Created by adama on 5/1/26.
//
#pragma once
#include "dsp/oscillator.h"
#include "dsp/adsr.h"
#include <stdbool.h>

typedef struct {
    Oscillator* carrier;
    Oscillator* modulator;
    ADSR* adsr;
    float* outBuffer;
    size_t bufferSize;
    bool on;
} Voice;

bool initVoice(Voice *v, Oscillator* carrier, Oscillator* modulator, ADSR* adsr, float* outBuffer, size_t bufferSize);
void voiceModulate(Voice* v, uint64_t releaseAt);