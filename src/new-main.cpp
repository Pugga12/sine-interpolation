#include <cstdlib>
#include "synth/VoiceManager.hpp"
#include "synth/MidiPreprocessor.hpp"

extern "C" {
	#include "dsp/wavetablegen.h"
    #include "io/wav.h"
}

std::vector<TimedEvent> create_voice_ladder_test() {
    std::vector<TimedEvent> events;

    // We will add 1 voice every 0.5 seconds (22050 samples)
    for (uint8_t v = 0; v < 8; ++v) {
        uint32_t startTime = v * 22050; 
        
        // We use a "Major 9th" chord spread so the frequencies are distinct
        // but close enough to rub together in the tanhf mixer.
        uint8_t pitches[] = {48, 52, 55, 59, 60, 64, 67, 71}; 
        
        events.push_back({v, startTime, NOTE_ON, pitches[v]});
    }

    for (int v = 0; v < 8; v++) {
        uint32_t endTime = 190000 + (v * 22050);

        events.push_back({v, endTime, NOTE_OFF, 0});
    }

    return events;
}

int main() {
	float* sinePtr = static_cast<float*>(malloc(sizeof(float) * 4096));
	float* outputPtr = static_cast<float*>(malloc(sizeof(float) * 2072700));
	wavetableGenSine(sinePtr, 4096);

    MidiProcessor mp;
    if(!mp.load("Minuet in G.mid")) {
        return -2;
    }
    mp.convert();

	VoiceManager vm(mp.getEvents(), sinePtr, sinePtr, 44100.0f, 4096);
	if (!vm.go(outputPtr, 2072700)) {
        std::printf("Invalid output size");
        return -1;
    }
    char* str = "new.wav";
    writeWavF32(str, outputPtr, 2028600, 44100);


	free(sinePtr);
	free(outputPtr);
	return 0;
}
