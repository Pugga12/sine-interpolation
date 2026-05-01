//
// Created by adama on 5/1/26.
//
#include "wavetablegen.h"

#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "constants.h"
#include <stdbool.h>

bool wtSine(int16_t* ptr, size_t length) {
    if (ptr == NULL || length == 0) {
        assert(0 && "wtSineDiscretize() called with invalid args");
        return false;
    }

    for (int i = 0; i < length; i++) {
        float theta = (2 * M_PI) * ((float)i / length);
        float a = sinf(theta);
        int16_t aInt = (int16_t)(a * 32767);
        ptr[i] = aInt;
    }

    return true;
}

// TODO: Rewrite to use polyBLEP because the fourier series causses es ar
bool wtSawBL(int16_t* table, size_t length, uint32_t maxHarmonics) {
    if (table == NULL || length == 0 || maxHarmonics == 0) {
        assert(0 && "wtSawBL() called with invalid args");
        return false;
    }

    for (int i = 0; i < length; i++) {
        float phase = (float)i / (float)length;
        float x = 0.0f;

        for (int k = 1; k <= maxHarmonics; k++) {
            x += sinf(2.0f * M_PI * phase * k) / (float)k;
        }

        x *= TWO_OVER_PI;

        table[i] = (int16_t)(x * 32767);
    }

    // wrap the table for sab
    table[length - 1] = table[0];
    return true;
}

bool wtTriangle(int16_t* table, size_t length) {
    if (table == NULL || length == 0) {
        assert(0 && "wtTriangle() called with invalid args");
        return false;
    }

    for (int i = 0; i < length; i++) {
        float phase = (float)i / (float)length;
        float a = 1.0f - 4.0f * fabsf(phase - 0.5f);

        int16_t aInt = (int16_t)(a * 32767);
        table[i] = aInt;
    }

    return true;
}