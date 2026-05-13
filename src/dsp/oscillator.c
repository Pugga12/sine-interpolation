/*
Copyright (C) 2026  Adam Aptowitz

This file is part of Dzsungel

Dzsungel is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Dzsungel.  If not, see <http://www.gnu.org/license>
*/
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
