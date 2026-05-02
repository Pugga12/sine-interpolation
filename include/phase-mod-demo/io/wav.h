#pragma once
#include <stdint.h>
#include <stdbool.h>
#define WAV_FORMAT_PCM 1
#define WAV_FORMAT_IEEE_FLOAT 3

bool writeWavS16(char *name, int16_t *data, size_t frameCount, uint32_t sampleRate);
bool writeWavF32(char *name, float *data, size_t frameCount, uint32_t sampleRate);