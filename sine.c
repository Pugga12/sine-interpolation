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

void wtSineRemap(int16_t* wtPtr, int16_t* outputPtr, size_t inputLength, size_t outputLength, float frequency, float sampleRate) {
    float accumulator = 0.0f;
    float phaseIncrement = (2048.0f * frequency) / sampleRate;
    for (int i = 0; i < outputLength; i++) {
        outputPtr[i] = linInterp(wtPtr, accumulator);
        accumulator += phaseIncrement;
        if (accumulator >= inputLength) accumulator -= inputLength;
    }
}

int main(int argc, char const *argv[])
{
    int16_t* wavetablePtr = (int16_t*)malloc(sizeof(int16_t) * 8192);
    int16_t* aNotePtr = (int16_t*)malloc(sizeof(int16_t) * 4096);

    if (wavetablePtr == NULL) {
        return -1;
    }

    wtSineDiscretize(wavetablePtr, 8192);
    wtSineRemap(wavetablePtr, aNotePtr, 8192, 4096, 20000.0f, 192000.0f);

    printf("i = [");
    for (int i = 0; i < 4096; i++) {
        printf("(%i, %i),", i, aNotePtr[i]);
    }
    printf("]\n");

    printf("j = [");
    for (int i = 0; i < 2048; i++) {
        printf("(%i, %i),", i, wavetablePtr[i]);
    }
    printf("]\n");

    return 0;
}
