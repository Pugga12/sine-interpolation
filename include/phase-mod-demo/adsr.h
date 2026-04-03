#pragma once
#include <fix16.h>
#include <stdbool.h>

enum ADSRState {
    IDLE,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
};

typedef struct 
{
    int state;
    float output;
    float attackTime;
    float releaseTime;
    float decayTime;
    float sustainLevel;
    bool gate; 
    bool prevGate;
    float sampleRate;
    uint32_t sampleCounter;
} ADSR;

void initADSR(ADSR* ptr, float attackTime, float releaseTime, float decayTime, float sustainLevel, float sampleRate);

void reset(ADSR* ptr);

float adsrCalculateLinear(ADSR* ptr);

void setGate(ADSR* adsr, bool gate);