//
// Created by adama on 5/1/26.
//
#pragma once
#include "synth/voice.h"
#include <stdlib.h>
#include <stdint.h>

int mix(Voice *vArr, size_t numVoices, float* outputBuffer, size_t outputSize);
