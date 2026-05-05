#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#define WAV_FORMAT_PCM 1
#define WAV_FORMAT_IEEE_FLOAT 3

/**
 * Generate a WAV file in the Signed 16-bit PCM format
 *
 * @param name Filename
 * @param data Pointer to samples
 * @param frameCount Number of samples in the input buffer
 * @param sampleRate Sample rate in Hz (typical is 44000 Hz)
 * @see writeWavF32() IEEE Float version of this function
 */
bool writeWavS16(char *name, int16_t *data, size_t frameCount, uint32_t sampleRate);
bool writeWavF32(char *name, float *data, size_t frameCount, uint32_t sampleRate);