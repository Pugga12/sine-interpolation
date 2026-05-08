#include <stdlib.h>
#include <stdio.h>
#include "dsp/adsr.h"
#include "dsp/oscillator.h"
#include "io/wav.h"
#include "dsp/wavetablegen.h"
#include <stdbool.h>
#include "synth/voice.h"
#include "synth/mixer.h"

static Oscillator o1C;
static Oscillator o1M;
static Oscillator o2C;
static Oscillator o2M;
static ADSR a1Amp;
static ADSR a2Amp;
static ADSR a1Mod;
static ADSR a2Mod;

static float* ob1;
static float* ob2;
static float* sinePtr;
static float* trianglePtr;

static Voice voices[2] = {};

void printPoints(float* values, size_t length) {
    printf("i = [");
    for (int i = 0; i < length; i++) {
        printf("(%i, %.9f)", i, values[i]);
        if (!(i == length - 1)) {
            printf(",");
        }
    }
    printf("]\n");
}

void initEverything() {
    oscInit(&o1C, sinePtr, WT_SIZE, 55.0f, 1, 44100.0f);
    oscInit(&o1M, sinePtr, WT_SIZE, 55.0f, 0.001f, 44100.0f);
    oscInit(&o2C, sinePtr, WT_SIZE, 261.63f, 1, 44100.0f);
    oscInit(&o2M, sinePtr, WT_SIZE, 3662.82f, 0.001f, 44100.0f);

    initADSR(&a1Amp, MS_TO_S(10), MS_TO_S(100), MS_TO_S(400), 0.5, 44100.0f);
    initADSR(&a2Amp, MS_TO_S(5), MS_TO_S(200), 1.5f, 0.2f, 44100.0f);
    initADSR(&a1Mod, MS_TO_S(2), MS_TO_S(100), MS_TO_S(150), 0.1f, 44100.0f);
    initADSR(&a2Mod, MS_TO_S(10), MS_TO_S(1), MS_TO_S(50), 0.0f, 44100.0f);

    initVoice(&voices[0], &o1C, &o1M, &a1Amp, ob1, 88200, &a1Mod);
    initVoice(&voices[1], &o2C, &o2M, &a2Amp, ob2, 88200, &a2Mod);

    voiceModulate(&voices[0], 22050);
    voiceModulate(&voices[1], 44100);
}

int main(int argc, char const *argv[])
{
    sinePtr = malloc(sizeof(float) * WT_SIZE);
    trianglePtr = malloc(sizeof(float) * WT_SIZE);
    ob1 = malloc(sizeof(float) * 88200);
    ob2 = malloc(sizeof(float) * 88200);
    float* obFinal = malloc(sizeof(float) * 88200);

    wavetableGenSine(sinePtr, WT_SIZE);
    wavetableGenTriangle(trianglePtr, WT_SIZE);
    initEverything();

    mix(voices, 2, obFinal, 88200);
    writeWavF32("demo-f32.wav", obFinal, 88200, 44100);

    free(sinePtr);
    free(obFinal);
    free(ob1);
    free(ob2);
    free(trianglePtr);
    return 0;
}
