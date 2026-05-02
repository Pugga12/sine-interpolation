//
// Created by adama on 5/1/26.
//
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define WT_SIZE 4096

bool wtSine(float *ptr, size_t length);
bool wtTriangle(float *table, size_t length);
bool wtSawBL(float *table, size_t length, uint32_t maxHarmonics);
