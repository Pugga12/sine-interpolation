//
// Created by adama on 5/1/26.
//
#include "synth/voice.h"
#include <stdbool.h>
#include <assert.h>
#include "dsp/adsr.h"
#include "math.h"
#include "dsp/wavetablegen.h"

bool initVoice(Voice *v, Oscillator* carrier, Oscillator* modulator, ADSR* ampAdsr, float* outBuffer, size_t bufferSize, ADSR* modAdsr) {
    if (!v || !carrier || !modulator || !ampAdsr || !modAdsr || !outBuffer || bufferSize == 0) {
        assert(0 && "Invalid arguments passed to initVoice()");
        return false;
    }

    v->carrier = carrier;
    v->modulator = modulator;
    v->ampAdsr = ampAdsr;
    v->modAdsr = modAdsr;
    v->bufferSize = bufferSize;
    v->outBuffer = outBuffer;
    v->on = true;

    return true;
}

float tableLinInterp(float* wtPtr, float x) {
    int i = (int)x;
    float a1 = wtPtr[i];
    float a2 = wtPtr[(i + 1) % WT_SIZE];
    float decimal = x - (float)i;
    return a1 + decimal * (a2 - a1);
}

static float bitCrush(const float x, const uint8_t precision) {
    const float max = (float)(1 << precision) - 1;
    const float max2 = max / 2;

    int quantitized = (int)roundf((x + 1) * max2);
    return (((float)quantitized / max) * 2) - 1;
}

void voiceModulate(Voice* v, uint64_t releaseAt) {
    setGate(v->ampAdsr, true);
    setGate(v->modAdsr, true);
    const float len = (float)v->carrier->tableLen;
    const float scalingConstant = (float)v->modulator->tableLen / 8;

    for (int i = 0; i < v->bufferSize; i++) {
        #ifdef EXPONENTIAL_ADSR
            const float ampEnv = adsrCalculateExp(v->ampAdsr);
            const float modEnv = adsrCalculateExp(v->modAdsr);
        #else
                const float ampEnv = adsrCalculateLinear(v->adsr);
        #endif
        const float modVal = v->modulator->table[(int)v->modulator->phase];
        const float qModVal = bitCrush(modVal, 8);
        const float currentModDepth = (v->modulator->modIndex * scalingConstant) * modEnv;

        // modulate the carrier by adding the phase deviation to the accumulator value
        float perturbed = v->carrier->phase + (qModVal * currentModDepth);

        if (i == releaseAt) {
            setGate(v->ampAdsr, false);
            setGate(v->modAdsr, false);
        }

        // phase accumulator wrapping
        perturbed -= len * floorf(perturbed / len);

        const float qPerturbation = bitCrush(perturbed, 8);
        v->outBuffer[i] = v->carrier->table[(int)qPerturbation] * ampEnv;

        // increase accumulators
        oscIncreasePhase(v->carrier);
        oscIncreasePhase(v->modulator);
    }
}
