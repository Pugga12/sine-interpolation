#pragma once
#include <stdint.h>
#include <stddef.h>

void writeWavMono(char* name, int16_t* data, size_t frameCount, uint32_t sampleRate);