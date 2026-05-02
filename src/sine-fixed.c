#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "oscillator.h"
#include "wav.h"
#include "fix16.h"
#include "constants.h"

void wtSineDiscretize_fixed16(fix16_t* ptr, int32_t length) {
    fix16_t lengthFixed = fix16_from_int(length);
    for (int i = 0; i < length; i++) {
        fix16_t theta = fix16_mul(
            TWO_PI_FIX16, 
            fix16_div(
                fix16_from_int(i),
                lengthFixed
            )
        );

        ptr[i] = fix16_sin(theta);
    }
}

fix16_t tableLinInterpF16(fix16_t* wtPtr, fix16_t x) {
    int integerPortion = fix16_to_int(x);
    fix16_t decimal = fix16_sub(x, fix16_from_int(integerPortion));
    fix16_t a1 = wtPtr[integerPortion];
    fix16_t a2 = wtPtr[integerPortion + 1];
    return fix16_add(
        a1,
        fix16_mul(
            decimal,
            fix16_sub(a2, a1) 
        )
    );
}

void wtFmModulate_fixed16(fix16_t* output, int32_t outputLength, OscillatorF16* carrier, OscillatorF16* modulator) {
    fix16_t tableLen = fix16_from_int(carrier->tableLen);
    fix16_t scalingConstant = fix16_div(fix16_from_int(modulator->tableLen), TWO_PI_FIX16);

    for (int i = 0; i < outputLength; i++) {
        fix16_t modVal = tableLinInterpF16(modulator->table, fix16_from_float(modulator->phase));

        fix16_t deviation = fix16_mul(
            fix16_from_float(modulator->modIndex),
            fix16_mul(modVal, scalingConstant)
        );

        fix16_t perturbed = fix16_add(
            fix16_from_float(carrier->phase),
            deviation
        );

        while (perturbed >= tableLen) perturbed = fix16_sub(perturbed, tableLen);
        while (perturbed < 0) perturbed = fix16_add(perturbed, tableLen);

        output[i] = tableLinInterpF16(carrier->table, perturbed);

        oscF16IncreasePhase(carrier);
        oscF16IncreasePhase(modulator);
    }
}

void fixed16ToInt16(int16_t* outputPtr, fix16_t* inputPtr, size_t len) {
    for (int i = 0; i < len; i++) {
        float floatVal = fix16_to_float(inputPtr[i]);
        outputPtr[i] = (int16_t)(floatVal * 32767.0f);
    }
}

int main(int argc, char const *argv[])
{
    fix16_t* wavetablePtr = (fix16_t*)malloc(sizeof(fix16_t) * 4096);
    fix16_t* modulatedWavePtr = (fix16_t*)malloc(sizeof(fix16_t) * 44100);

    wtSineDiscretize_fixed16(wavetablePtr, 4096);

    OscillatorF16 mainOscillator;
    oscInitF16(&mainOscillator, wavetablePtr, 4096, 261.63f, 1, 44100.0f);

    OscillatorF16 modulatorOscillator;
    oscInitF16(&modulatorOscillator, wavetablePtr, 4096, 392.445f, 10.0f, 44100.0f);

    wtFmModulate_fixed16(modulatedWavePtr, 44100, &mainOscillator, &modulatorOscillator);

    int16_t* wavPtr = (int16_t*)malloc(sizeof(int16_t) * 44100);

    fixed16ToInt16(wavPtr, modulatedWavePtr, 44100);
    writeWavS16("demo-fix.wav", wavPtr, 44100, 44100);

    return 0;
}
