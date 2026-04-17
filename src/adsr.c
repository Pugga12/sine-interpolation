#include "adsr.h"
#include <assert.h>

void initADSR(ADSR* ptr, float attackTime, float releaseTime, float decayTime, float sustainLevel, float sampleRate) {
    ptr->state = IDLE;
    ptr->output = 0.0f;
    ptr->attackTime = attackTime;
    ptr->decayTime = decayTime;
    ptr->releaseTime = releaseTime;
    ptr->sustainLevel = sustainLevel;
    ptr->gate = false;
    ptr->prevGate = false;
    ptr->sampleRate = sampleRate;
}

void reset(ADSR* ptr) {
    ptr->state = IDLE;
    ptr->output = 0.0f;
    ptr->gate = false;
    ptr->sampleCounter = 0;
}

void setGate(ADSR* adsr, bool gate) {
    adsr->gate = gate;
}

static inline float linInterp(float b, float a2, float a1, float x) {
    return ((a2 - a1) * x) + b; // y = (a1-a2)x + b
}

float adsrCalculateLinear(ADSR* ptr) {
    float timeElapsed = ptr->sampleCounter / ptr->sampleRate;

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
            float attackSeconds = ptr->attackTime / 1000.0f;

            if (timeElapsed >= attackSeconds) {
                ptr->state = DECAY;
                ptr->sampleCounter = 0;
                return 1.0f;
            }
            ptr->output = timeElapsed / attackSeconds;
            break;
        case DECAY:
            float decaySeconds = ptr->decayTime / 1000.0f;

            if (timeElapsed >= decaySeconds) {
                ptr->state = SUSTAIN;
                ptr->sampleCounter = 0;
                return ptr->sustainLevel;
            }
            float decayProgress = timeElapsed / decaySeconds;
            ptr->output = linInterp(1.0f, ptr->sustainLevel, 1.0f, decayProgress);
            break;
        case SUSTAIN:
            ptr->output = ptr->sustainLevel;
            break;
        case RELEASE:
            float releaseSconds = ptr->releaseTime / 1000.0f;
            if (timeElapsed >= releaseSconds) {
                ptr->state = IDLE;
                return 0.0f;
            }
            float releaseProgress = timeElapsed / releaseSconds;
            ptr->output = ptr->sustainLevel * (1.0f - releaseProgress);
            break;
        case IDLE:
            break;
        default:
            assert(0 && "Invalid ADSR state");
    }

    ptr->sampleCounter++;
    ptr->prevGate = ptr->gate;
    return ptr->output;
}