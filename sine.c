#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

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

int main(int argc, char const *argv[])
{
    int16_t* wavetablePtr = (int16_t*)malloc(sizeof(int16_t) * 256);
    int16_t* aNotePtr = (int16_t*)malloc(sizeof(int16_t) * 4096);

    if (wavetablePtr == NULL) {
        return -1;
    }

    wtSineDiscretize(wavetablePtr, 255);
    wtSineRemap(wavetablePtr, aNotePtr, 255, 4096, 2600.0f, 44100.0f);

    printf("i = [");
    for (int i = 0; i < 4096; i++) {
        printf("(%i, %i),", i, aNotePtr[i]);
    }
    printf("]\n");

    printf("j = [");
    for (int i = 0; i < 256; i++) {
        printf("(%i, %i),", i, wavetablePtr[i]);
    }
    printf("]\n");

    free(wavetablePtr);
    free(aNotePtr);
    return 0;
}
