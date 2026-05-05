//
// Created by adama on 5/1/26.
//
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define WT_SIZE 4096

/**
 * Generate a sine wavetable with n points
 *
 * @param ptr Output buffer
 * @param length Number of points to generate
 */
bool wavetableGenSine(float *ptr, size_t length);

/**
 * Generate a triangle wavetable with n points
 *
 * @param ptr Output buffer
 * @param length Number of points to generate
 */
bool wavetableGenTriangle(float *table, size_t length);

/**
 * Band-limited sine wave with n length and k harmonics using a fourier series
 *
 * @param ptr Output buffer
 * @param Number of points to generate
 * @param Maximum harmonics used for the fourier series
 * @deprecated This function is deprecated and should not be used. A polyBLEP SM implementation of this function will be created later.
 */
bool wavetableGenBLSaw(float *table, size_t length, uint32_t maxHarmonics);
