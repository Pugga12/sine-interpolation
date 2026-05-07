#include "synth/SynthVoice.hpp"
#include <cmath>
#include <cstdint>

extern "C" {
#include "dsp/adsr.h"
#include "dsp/oscillator.h"
}

static float noteToFrequency(uint32_t note) {
	return 440.0f * std::pow(2, (note - 69) / 12);
}

void SynthVoice::init(Program *program, float *modTable, float *carrierTable, float sr, size_t tableSize) {
	oscInit(&carrier, carrierTable, tableSize, 55.0f, 1, sr);
	oscInit(&modulator, modTable, tableSize, 82.5f, 0.001f, sr);

	envStructToAdsr(&ampEnv, &program->ampEnv, sr);
	envStructToAdsr(&modEnv, &program->ampEnv, sr);
}

void SynthVoice::noteOn(uint32_t midiNote) {
	this->currentMidiNote = midiNote;
	const float newFrequency = noteToFrequency(midiNote);

	oscUpdateFrequency(&carrier, newFrequency);
	oscUpdateFrequency(&modulator, newFrequency * 1.5f);

	reset(&ampEnv);
	reset(&modEnv);
	setGate(&ampEnv, true);
	setGate(&modEnv, true);
}

void SynthVoice::noteOff() {
	setGate(&ampEnv, false);
	setGate(&modEnv, false);
}

static float bitCrush(const float x, const uint8_t precision) {
	const float max = (float)(1 << precision) - 1;
	const float max2 = max / 2;

	int quantitized = (int)roundf((x + 1) * max2);
	return (((float)quantitized / max) * 2) - 1;
}

void SynthVoice::renderInner(uint32_t start, uint32_t end, float* outputBuffer) {
	const float len = static_cast<float>(carrier.tableLen);
	const float scalingConstant = static_cast<float>(modulator.tableLen) / 8;

	for (uint32_t i = start; i < end; i++) {
		const float ampEnvVal = adsrCalculateExp(&ampEnv);
		const float modEnvVal = adsrCalculateExp(&modEnv);

		const float modVal = modulator.table[static_cast<int>(modulator.phase)];

		const float qModVal = bitCrush(modVal, 8);
		const float currentModDepth = (modulator.modIndex * scalingConstant) * modEnvVal;

		float perturbed = carrier.phase + (qModVal * currentModDepth);

		perturbed -= len * floorf(perturbed / len);

		const float qPerturbation = bitCrush(perturbed, 8);
		outputBuffer[i] += carrier.table[static_cast<int>(qPerturbation)] * ampEnvVal; 

		oscIncreasePhase(&carrier);
		oscIncreasePhase(&modulator);
	}	
}

void SynthVoice::processBlock(float* outputBuffer, size_t blockSize) {
	if (state == VOICE_IDLE) return;
	uint32_t cursor = 0;

	while (eventIndex < events.size()) {
		VoiceEvent& ev = events[eventIndex];
		renderInner(cursor, ev.offset, outputBuffer);

		cursor = ev.offset;

		if (ev.type == EventType::NOTE_ON) {
			noteOn(ev.val);
		} else if (ev.type == EventType::NOTE_OFF){
			noteOff();
		}

		eventIndex++;
	}

	renderInner(cursor, blockSize, outputBuffer);
	events.clear();
	eventIndex = 0;
}

void SynthVoice::queueBlockEvents(VoiceEvent *arr, size_t n) {
	events.assign(arr, arr+n);
	eventIndex = 0;
}
