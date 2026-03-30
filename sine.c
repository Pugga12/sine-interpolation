#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void wtGenSine(int16_t* ptr, size_t length) {
    for (int i = 0; i < length; i++) {
        float theta = (2 * M_PI) * ((float)i / length);
        float a = sinf(theta);
        int16_t aInt = (int16_t)(a * 32767);
        ptr[i] = aInt;
    }
}

int main(int argc, char const *argv[])
{
    int16_t* wavetablePtr = (int16_t*)malloc(sizeof(int16_t) * 2048);

    if (wavetablePtr == NULL) {
        return -1;
    }

    wtGenSine(wavetablePtr, 2048);

    printf("i = [");
    for (int i = 0; i < 2048; i++) {
        printf("(%i, %i),", i, wavetablePtr[i]);
    }
    printf("]");

    return 0;
}
