#include "dsp/adsr.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/// small number for comparisons
#define EPSILON 1e-4

// === MATH ===
static inline float linInterp(float b, float a2, float a1, float x) {
    return ((a2 - a1) * x) + b; // y = (a1-a2)x + b
}

static float calculateExpRatio(float t, float sampleRate) {
    uint32_t n = (uint32_t)(t*sampleRate);
    float exponent = log(EPSILON) / n;
    return (float) 1 - exp(exponent);
}

// === ADSR Functions ===

void cacheRatios(ADSR* adsr) {
    adsr->rA = calculateExpRatio(adsr->attackSeconds, adsr->sampleRate);
    adsr->rD = calculateExpRatio(adsr->decaySeconds, adsr->sampleRate);
    adsr->rR = calculateExpRatio(adsr->releaseSeconds, adsr->sampleRate);
}

bool initADSR(ADSR* ptr, float attackSeconds, float releaseSeconds, float decaySeconds, float sustainLevel, uint32_t sampleRate) {
    // divide by zero checks at init (time vals cannot be less then 0)
    if (attackSeconds <= 0 || decaySeconds <= 0 || releaseSeconds <= 0) {
        assert(0 && "Invalid time values passed to ADSR creation");
        return false;
    }

    ptr->state = IDLE;
    ptr->attackSeconds = attackSeconds;
    ptr->decaySeconds = decaySeconds;
    ptr->releaseSeconds = releaseSeconds;
    ptr->sustainLevel = sustainLevel;
    ptr->gate = false;
    ptr->prevGate = false;
    ptr->sampleRate = sampleRate;
    ptr->rA = 0.0f;
    ptr->rD = 0.0f;
    ptr->rR = 0.0f;
    ptr->y = 0.0f;
    cacheRatios(ptr);
    return true;
}

bool envStructToAdsr(ADSR* ptr, Envelope* env, float sampleRate) {
    return initADSR(ptr, env->attackTime, env->releaseTime, env->decayTime, env->sustainLevel, (uint32_t)sampleRate);
}

void reset(ADSR* ptr) {
    ptr->state = IDLE;
    ptr->gate = false;
    ptr->sampleCounter = 0;
}

void setGate(ADSR* adsr, bool gate) {
    adsr->gate = gate;
}

float adsrCalculateLinear(ADSR* ptr) {
    float timeElapsed = ptr->sampleCounter / (float)ptr->sampleRate;
    float output = 0.0f;

    if (ptr->gate && !ptr->prevGate) {
        ptr->state = ATTACK;
        ptr->sampleCounter = 0;
    }
    if (!ptr->gate && ptr->prevGate) {
        ptr->state = RELEASE;
        ptr->sampleCounter = 0;
    }

    switch (ptr->state) {
        case ATTACK:
            output = timeElapsed / ptr->attackSeconds;
            if (timeElapsed >= ptr->attackSeconds) {
                ptr->state = DECAY;
                ptr->sampleCounter = 0;
                output = 1.0f;
            }
            break;
        case DECAY:
            float decayProgress = timeElapsed / ptr->decaySeconds;
            output = linInterp(1.0f, ptr->sustainLevel, 1.0f, decayProgress);
            if (timeElapsed >= ptr->decaySeconds) {
                ptr->state = SUSTAIN;
                ptr->sampleCounter = 0;
                output = ptr->sustainLevel;
            }
            break;
        case SUSTAIN:
            return ptr->sustainLevel;
        case RELEASE:
            float releaseProgress = timeElapsed / ptr->releaseSeconds;
            output = ptr->sustainLevel * (1.0f - releaseProgress);
            if (timeElapsed >= ptr->releaseSeconds) {
                ptr->state = IDLE;
                output = 0.0f;
            }
            break;
        case IDLE:
            return 0.0f;
        default:
            assert(0 && "Invalid ADSR state");
            return -1.0f;
    }

    ptr->sampleCounter++;
    ptr->prevGate = ptr->gate;
    return output;
}

float adsrCalculateExp(ADSR* adsr) {
    if (adsr->gate && !adsr->prevGate) {
        adsr->state = ATTACK;
//        cacheRatios(adsr);
        adsr->sampleCounter = 0;
        adsr->y = 0.0f;
    }
    if (!adsr->gate && adsr->prevGate) {
        adsr->state = RELEASE;
        adsr->sampleCounter = 0;
    }

    switch (adsr->state) {
        case ATTACK:
            adsr->y += (1.0f - adsr->y) * adsr->rA;
            if (adsr->y >= 1.0f - EPSILON) {
                adsr->state = DECAY;
                adsr->sampleCounter = 0;
                adsr->y = 1.0f;
            }
            break;
        case DECAY:
            adsr->y += (adsr->sustainLevel - adsr->y) * adsr->rD;
            if (adsr->y <= adsr->sustainLevel + EPSILON) {
                adsr->state = SUSTAIN;
                adsr->sampleCounter = 0;
                adsr->y = adsr->sustainLevel;
            }
            break;
        case SUSTAIN:
            break;
        case RELEASE:
            adsr->y *= (1.0f - adsr->rR);
            if (adsr->y <= EPSILON) {
                adsr->y = 0.0f;
                adsr->state = IDLE;
            }
            break;
        case IDLE:
            return 0.0f;
        default:
            assert(0 && "Invalid ADSR state");
            return -1.0f;
    }

    adsr->sampleCounter++;
    adsr->prevGate = adsr->gate;
    return adsr->y;
}

float adsrGetAtSample(ADSR* adsr, uint32_t sampleNo) {
    
}
