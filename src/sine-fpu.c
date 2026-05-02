#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "adsr.h"
#include "oscillator.h"
#include "wav.h"
#include "constants.h"
#include <assert.h>
#include "wavetablegen.h"
#include <stdbool.h>

#define MS_TO_S(ms) (ms / 1000.0f)

float tableLinInterp(float* wtPtr, float x) {
    int i = (int)x;
    float a1 = wtPtr[i];
    float a2 = wtPtr[i + 1];
    float decimal = x - (float)i;
    return a1 + decimal * (a2 - a1);
}

/**
 * Uses phase modulation to modulate a carrier.
 * This makes use of phase modulation, in which the phase of the carrier is offset by the value of the modulator at that point in time
 * 
 * @param output The output buffer to write the synthesized samples to
 * @param outputLength The number of samples to generate
 * @param carrier The oscillator whose frequency will be modulated
 * @param modulator The oscillator that controls the modulation depth
 */
void wtFmModulate(float* output, size_t outputLength, Oscillator* carrier, Oscillator* modulator, ADSR* adsr) {
    setGate(adsr, true);

    for (int i = 0; i < outputLength; i++) {
        float modVal = tableLinInterp(modulator->table, modulator->phase);
        
        float scalingConstant = modulator->tableLen / (2.0f * M_PI);
        float phaseDeviation = modulator->modIndex * modVal * scalingConstant;

        // modulate the carrier by adding the phase deviation to the accumulator value
        float perturbed = carrier->phase + phaseDeviation;

        if (i ==  77170) {
            setGate(adsr, false);
        }

        // phase accumulator wrapping
        perturbed = fmodf(perturbed, (float)carrier->tableLen);
        while (perturbed < 0) perturbed += carrier->tableLen;

        #ifdef EXPONENTIAL_ADSR
            float amplitude = adsrCalculateExp(adsr);
        #else
            float amplitude = adsrCalculateLinear(adsr);
        #endif
        float outputVal = tableLinInterp(carrier->table, perturbed);
        output[i] = outputVal * amplitude;

        // increase accumulators
        oscIncreasePhase(carrier);
        oscIncreasePhase(modulator);
    }
}

void printPoints(float* values, size_t length) {
    printf("i = [");
    for (int i = 0; i < length; i++) {
        printf("(%i, %.9f)", i, values[i]);
        if (!(i == length - 1)) {
            printf(",");
        }
    }
    printf("]\n");
}

int main(int argc, char const *argv[])
{
    float* sinePtr = malloc(sizeof(float) * WT_SIZE);
    float* trianglePtr = malloc(sizeof(float) * WT_SIZE);
    float* modulatedWavePtr = malloc(sizeof(float) * 88200);

    wtSine(sinePtr, WT_SIZE);
    wtTriangle(trianglePtr, WT_SIZE);

    Oscillator mainOscilator;
    oscInit(&mainOscilator, sinePtr, 4096, 261.63f, 1, 44100.0f);

    Oscillator modulatorOscillator;
    oscInit(&modulatorOscillator, trianglePtr, 4096, 392.445f, 0.5f, 44100.0f);

    ADSR adsr; 
    bool adsrValid = initADSR(&adsr, MS_TO_S(50.0f), MS_TO_S(100.0f), MS_TO_S(50.0f), 0.25f, 44100);
    if (!adsrValid) {
        return -1;
    }

    wtFmModulate(modulatedWavePtr, 88200, &mainOscilator, &modulatorOscillator, &adsr);
    
//    printPoints(modulatedWavePtr, 4096);
    printPoints(trianglePtr, 4096);
    printPoints(sinePtr, 4096);
    writeWavF32("demo-f32.wav", modulatedWavePtr, 88200, 44100);

    free(sinePtr);
    free(modulatedWavePtr);
    free(trianglePtr);
    return 0;
}
