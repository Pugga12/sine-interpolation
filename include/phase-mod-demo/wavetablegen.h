//
// Created by adama on 5/1/26.
//
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define WT_SIZE 4096

bool wtSine(int16_t* ptr, size_t length);
bool wtTriangle(int16_t* table, size_t length);
bool wtSawBL(int16_t* table, size_t length, uint32_t maxHarmonics);

typedef struct {
    int16_t* sineTbl;
    int16_t* saw110;
    int16_t* saw220;
    int16_t* saw440;
    int16_t* saw880;
    int16_t* triangle;
} TableRefs;