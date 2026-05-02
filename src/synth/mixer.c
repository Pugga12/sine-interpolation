//
// Created by adama on 5/1/26.
//
#include "synth/mixer.h"

#include <math.h>

#include "synth/voice.h"
#include <stdlib.h>
#include <stdint.h>

static int whitelist(Voice *vArr, Voice *whitelistedArr[], size_t numVoices, size_t minSize) {
    int voicesAccepted = 0;

    for (int i = 0; i < numVoices; i++) {
        if (vArr[i].bufferSize >= minSize && vArr[i].on) {
            whitelistedArr[voicesAccepted++] = &vArr[i];
        }
    }
    return voicesAccepted;
}

int mix(Voice *vArr, size_t numVoices, float* outputBuffer, size_t outputSize) {
    Voice *whitelistedVoices[numVoices];
    int accepted = whitelist(vArr, whitelistedVoices, numVoices, outputSize);

    for (int i = 0; i < outputSize; i++) {
        float accumulator = 0.0f;
        for (int j = 0; j < accepted; j++) {
            accumulator += vArr[j].outBuffer[i];
        }

        float gain = 0.5f;
        float y = accumulator * gain;

        outputBuffer[i] = tanhf(y);
    }

    return accepted;
}