//
// Created by adama on 5/1/26.
//
#include "synth/voice.h"
#include <stdbool.h>
#include <assert.h>
#include "constants.h"
#include "math.h"

bool initVoice(Voice *v, Oscillator* carrier, Oscillator* modulator, ADSR* adsr, float* outBuffer, size_t bufferSize) {
    if (!v || !carrier || !modulator || !adsr || !outBuffer || bufferSize == 0) {
        assert(0 && "Invalid arguments passed to initVoice().l.");
        return false;
    }

    v->carrier = carrier;
    v->modulator = modulator;
    v->adsr = adsr;
    v->bufferSize = bufferSize;
    v->outBuffer = outBuffer;
    v->on = true;

    return true;
}

float tableLinInterp(float* wtPtr, float x) {
    int i = (int)x;
    float a1 = wtPtr[i];
    float a2 = wtPtr[i + 1];
    float decimal = x - (float)i;
    return a1 + decimal * (a2 - a1);
}


void voiceModulate(Voice* v, uint64_t releaseAt) {
    setGate(v->adsr, true);

    for (int i = 0; i < v->bufferSize; i++) {
        float modVal = tableLinInterp(v->modulator->table, v->modulator->phase);

        float scalingConstant = v->modulator->tableLen / (2.0f * M_PI);
        float phaseDeviation = v->modulator->modIndex * modVal * scalingConstant;

        // modulate the carrier by adding the phase deviation to the accumulator value
        float perturbed = v->carrier->phase + phaseDeviation;

        if (i == releaseAt) {
            setGate(v->adsr, false);
        }

        // phase accumulator wrapping
        perturbed = fmodf(perturbed, (float)v->carrier->tableLen);
        while (perturbed < 0) perturbed += v->carrier->tableLen;

#ifdef EXPONENTIAL_ADSR
        float amplitude = adsrCalculateExp(v->adsr);
#else
        float amplitude = adsrCalculateLinear(adsr);
#endif
        float outputVal = tableLinInterp(v->carrier->table, perturbed);
        v->outBuffer[i] = outputVal * amplitude;

        // increase accumulators
        oscIncreasePhase(v->carrier);
        oscIncreasePhase(v->modulator);
    }
}