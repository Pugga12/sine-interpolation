#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "wav.h"
void writeWavMono(char* name, int16_t* data, size_t frameCount, uint32_t sampleRate) {
    uint32_t duration = frameCount / sampleRate;
    uint16_t chanNumber = 1;
    uint16_t bits = 16;
    uint32_t length = (frameCount * chanNumber * bits) / 8;

    FILE *fp = fopen(name, "wb");
    
    if (fp == NULL) {
        printf("File %s could not be opened", name);
        return;
    }

    //// WAVE Header Data
    fwrite("RIFF", 1, 4, fp);
    uint32_t chunk_size = length + 44 - 8;
    fwrite(&chunk_size, 4, 1, fp);
    fwrite("WAVE", 1, 4, fp);
    fwrite("fmt ", 1, 4, fp);
    uint32_t subchunk1_size = 16;
    fwrite(&subchunk1_size, 4, 1, fp);
    uint16_t fmt_type = 1;  // 1 = PCM
    fwrite(&fmt_type, 2, 1, fp);
    fwrite(&chanNumber, 2, 1, fp);
    fwrite(&sampleRate, 4, 1, fp);
    // (Sample Rate * BitsPerSample * Channels) / 8
    uint32_t byte_rate = sampleRate * bits * chanNumber / 8;
    fwrite(&byte_rate, 4, 1, fp);
    uint16_t block_align = chanNumber * bits / 8;
    fwrite(&block_align, 2, 1, fp);
    fwrite(&bits, 2, 1, fp);

    // Marks the start of the data
    fwrite("data", 1, 4, fp);
    fwrite(&length, 4, 1, fp);  // Data size

    for (int i = 0; i < frameCount; i++) {
        fwrite(&data[i], 2, 1, fp);
    }

    fclose(fp);
}