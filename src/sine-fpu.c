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

int16_t tableLinInterp(int16_t* wtPtr, float x) {
    int i = (int)x;
    int16_t a1 = wtPtr[i];
    int16_t a2 = wtPtr[i + 1];
    float decimal = x - i;
    return (int16_t)(a1 + (decimal * (a2 - a1)));
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
void wtFmModulate(int16_t* output, size_t outputLength, Oscillator* carrier, Oscillator* modulator, ADSR* adsr) {
    setGate(adsr, true);

    for (int i = 0; i < outputLength; i++) {
        float modVal = tableLinInterp(modulator->table, modulator->phase);
        
        float scalingConstant = modulator->tableLen / (2.0f * M_PI);
        float phaseDeviation = modulator->modIndex * (modVal / 32767.0f) * scalingConstant;

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
        output[i] = (int16_t)(outputVal * amplitude);

        // increase accumulators
        oscIncreasePhase(carrier);
        oscIncreasePhase(modulator);
    }
}

void printPoints(int16_t* values, size_t length) {
    printf("i = [");
    for (int i = 0; i < length; i++) {
        printf("(%i, %i)", i, values[i]);
        if (!(i == length - 1)) {
            printf(",");
        }
    }
    printf("]\n");
}

int main(int argc, char const *argv[])
{
    int16_t* sinePtr = (int16_t*)malloc(sizeof(int16_t) * WT_SIZE);
    int16_t* trianglePtr = (int16_t*)malloc(sizeof(int16_t) * WT_SIZE);
    int16_t* modulatedWavePtr = (int16_t*)malloc(sizeof(int16_t) * 88200);

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
    writeWavMono("demo.wav", modulatedWavePtr, 88200, 44100);

    free(sinePtr);
    free(modulatedWavePtr);
    free(trianglePtr);
    return 0;
}
