#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MS_TO_S(ms) (ms / 1000.0f)

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
    float attackSeconds;
    float releaseSeconds;
    float decaySeconds;
    float sustainLevel;
    bool gate; 
    bool prevGate;
    uint32_t sampleRate;
    uint32_t sampleCounter;

    float rA;
    float rD;
    float rR;
    float y;
} ADSR;

typedef struct {
    float attackTime;
    float decayTime;
    float sustainLevel;
    float releaseTime;
} Envelope;

/**
 * Initialize the ADSR struct
 *
 * @param ptr Pointer to ADSR struct
 * @param attackSeconds Attack time, in seconds (must be >=0)
 * @param releaseSeconds Release time, in seconds (must be >=0)
 * @param decaySeconds Decay time, in seconds (must be >= 0)
 * @param sustainLevel The sustain amplitude, from 0.0 to 1.0
 * @param sampleRate Sample rate of the audio, in Hz. A typical value is 44100 Hz
 * @returns true if successfully created, false if parameters are invalid
 */
bool initADSR(ADSR* ptr, float attackSeconds, float releaseSeconds, float decaySeconds, float sustainLevel, uint32_t sampleRate);

bool envStructToAdsr(ADSR* ptr, Envelope* env, float sampleRate);

/// Resets the ADSR state machine
void reset(ADSR* ptr);

/**
 * Calculates the current ADSR value using linear functions.
 * @par It is recommended to use @link adsrCalculateExp instead, as it is more realistic
 * @par This must be called once per sample calculated
 *
 * @param ptr Pointer to ADSR struct    
 * @returns ADSR amplitude, from 0.0 to 1.0
 */
float adsrCalculateLinear(ADSR* ptr);

/// Sets the gate of the ADSR state machine
void setGate(ADSR* adsr, bool gate);

/**
 * Calculates the current ADSR with a logarthmic curve for ATTACK and exponential for DECAY/RELEASE
 *
 * @param adsr Pointer to ADSR struct
 * @returns ADSR amplitude, from 0.0 to 1.0
 */
float adsrCalculateExp(ADSR* adsr);
