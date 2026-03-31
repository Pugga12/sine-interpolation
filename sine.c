#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "oscillator.h"
#include "wav.h"

void wtSineDiscretize(int16_t* ptr, size_t length) {
    for (int i = 0; i < length; i++) {
        float theta = (2 * M_PI) * ((float)i / length);
        float a = sinf(theta);
        int16_t aInt = (int16_t)(a * 32767);
        ptr[i] = aInt;
    }
}

int16_t linInterp(int16_t* wtPtr, float x) {
    int i = (int)x;
    int16_t a1 = wtPtr[i];
    int16_t a2 = wtPtr[i + 1];
    float decimal = x - i;
    return (int16_t)(a1 + (decimal * (a2 - a1)));
}

/**
 * Generates a new wavetable at a certain frequency, sample rate, and output length 
 * 
 * @param wtPtr The pointer to the wavetable to sample and interpolate from
 * @param outputPtr The pointer to the output buffer. The new wavetable will be written here
 * @param inputLength Number of input points
 * @param outputLength Number of output points
 * @param frequency The frequency of the sine wave to be created
 * @param sampleRate How often we sample from the wavetable. This influences our phase accumulator increment and thus how many samples there are in a single cycle.
 */
void wtSineRemap(int16_t* wtPtr, int16_t* outputPtr, size_t inputLength, size_t outputLength, float frequency, float sampleRate) {
    float accumulator = 0.0f;
    float phaseIncrement = (inputLength * frequency) / sampleRate;
    printf("Phase accumulator increment %f samples\n", phaseIncrement);
    for (int i = 0; i < outputLength; i++) {
        outputPtr[i] = linInterp(wtPtr, accumulator);
        accumulator += phaseIncrement;
        if (accumulator >= inputLength) accumulator -= inputLength;
    }
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
void wtFmModulate(int16_t* output, size_t outputLength, Oscillator* carrier, Oscillator* modulator) {
    for (int i = 0; i < outputLength; i++) {
        float modVal = linInterp(modulator->table, modulator->phase);
        
        // remap mod value on scale of [-1, 1], and use it to calculate the desired frequency deviation. modIndex is adjusted by the modulator frequency to keep the ratios consistent
        float freqDeviation = modulator->modIndex * modulator->oscillatorFrequency * (modVal / 32767.0f);
        
        // map frequency deviation to a phase accumulator deviation
        float phaseDeviation = (freqDeviation * modulator->tableLen) / modulator->sampleRate;

        // modulate the carrier by adding the phase deviation to the accumulator value
        float perturbed = carrier->phase + phaseDeviation;

        // phase accumulator wrapping
        perturbed = fmodf(perturbed, (float)carrier->tableLen);
        while (perturbed < 0) perturbed += carrier->tableLen;

        // interpolate to get modulated carrier value; store to outp
        output[i] = linInterp(carrier->table, perturbed);

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
    int16_t* wavetablePtr = (int16_t*)malloc(sizeof(int16_t) * 4096);
    int16_t* modulatedWavePtr = (int16_t*)malloc(sizeof(int16_t) * 44100);

    wtSineDiscretize(wavetablePtr, 4096);

    Oscillator mainOscilator;
    oscInit(&mainOscilator, wavetablePtr, 4096, 440.0f, 1, 44100.0f);

    Oscillator modulatorOscillator;
    oscInit(&modulatorOscillator, wavetablePtr, 4096, 1980.0f, 5.0f, 44100.0f);

    wtFmModulate(modulatedWavePtr, 44100, &mainOscilator, &modulatorOscillator);
    
//    printPoints(modulatedWavePtr, 4096);
    writeWavMono("demo.wav", modulatedWavePtr, 44100, 44100);

    free(wavetablePtr);
    free(modulatedWavePtr);
    return 0;
}
