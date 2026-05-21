/*
Copyright (C) 2026  Adam Aptowitz

This file is part of Dzsungel

Dzsungel is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Dzsungel.  If not, see <http://www.gnu.org/license>
*/
#include "synth/SynthVoice.hpp"
#include <cmath>
#include <cstdint>
#include "constants.h"

extern "C" {
#include "dsp/adsr.h"
#include "dsp/oscillator.h"
}

static float noteToFrequency(uint32_t note) {
	return 440.0f * std::exp2((note - 69.0f) / 12.0f);
}

void SynthVoice::init(Program &program, float *modTable, float *carrierTable, float sr, size_t tableSize) {
	oscInit(&carrier, carrierTable, tableSize, 55.0f, 1, sr);
	oscInit(&modulator, modTable, tableSize, 82.5f, program.modIndex, sr);

	envStructToAdsr(&ampEnv, &program.ampEnv, sr);
	envStructToAdsr(&modEnv, &program.ampEnv, sr);
	cToMRatio = program.cToMRatio;
	type = program.type;
	defaultProgram = &program;
}

void SynthVoice::noteOn(uint32_t midiNote, uint32_t velocity) {
	this->currentMidiNote = midiNote;
	baseCarrier = noteToFrequency(midiNote);
	currentCarrierFrequency = baseCarrier;
	targetCarrierFrequency = baseCarrier;
	rampSamplesRemaining = 0;

	oscUpdateFrequency(&carrier, baseCarrier);
	oscUpdateFrequency(&modulator, baseCarrier * cToMRatio);

	reset(&ampEnv);
	reset(&modEnv);
	setGate(&ampEnv, true);
	setGate(&modEnv, true);
	state = VOICE_ACTIVE;
}

void SynthVoice::noteOff() {
	setGate(&ampEnv, false);
	setGate(&modEnv, false);
	state = VOICE_RELEASING;
}

static float bitCrush(const float x, const uint8_t precision) {
	const float max = (float)(1 << precision) - 1;
	const float max2 = max / 2;

	int quantitized = (int)roundf((x + 1) * max2);
	return (((float)quantitized / max) * 2) - 1;
}

void SynthVoice::renderInnerNormal(uint32_t start, uint32_t end, float* outputBuffer) {
	if (end - start <= 0) {
		return;
	}

	const float len = static_cast<float>(carrier.tableLen);
	const float invLen = 1.0f / len;
	const float scalingConstant = len / 8;

	for (uint32_t i = start; i < end; i++) {
		if (rampSamplesRemaining > 0) {
			currentCarrierFrequency += rampInc;

			oscUpdateFrequency(&carrier, currentCarrierFrequency);
			oscUpdateFrequency(&modulator, currentCarrierFrequency * cToMRatio);

			rampSamplesRemaining--;
			if (rampSamplesRemaining == 0) {
				currentCarrierFrequency = targetCarrierFrequency;
				oscUpdateFrequency(&carrier, targetCarrierFrequency);
				oscUpdateFrequency(&modulator, targetCarrierFrequency * cToMRatio);
			}
		}

		const float ampEnvVal = adsrCalculateExp(&ampEnv);
		const float modEnvVal = adsrCalculateExp(&modEnv);

		if (ampEnv.state == IDLE) {
			state = VOICE_IDLE;
			break;
		}

		const float modVal = modulator.table[static_cast<int>(modulator.phase)];

		const float currentModDepth = (modulator.modIndex * scalingConstant) * modEnvVal;

		float perturbed = carrier.phase + (modVal * currentModDepth);
		if (perturbed < 0) perturbed += len;

		perturbed -= len * (float)((int)(perturbed * invLen));
		const float vol = ampEnvVal *  expresssion * masterVolume;

		outputBuffer[i] += carrier.table[static_cast<int>(perturbed)] * vol;

		oscIncreasePhase(&carrier);
		oscIncreasePhase(&modulator);
	}	
}

void SynthVoice::renderInnerFeedback(uint32_t start, uint32_t end, float* outputBuffer) {
	if (end - start <= 0) {
		return;
	}

	const float len = static_cast<float>(carrier.tableLen);
	const float invLen = 1.0f / len;
	const float scalingConstant = len / TWO_PI_FLOAT;

	for (int i = start; i < end; i++) {
		if (rampSamplesRemaining > 0) {
			currentCarrierFrequency += rampInc;

			oscUpdateFrequency(&carrier, currentCarrierFrequency);

			rampSamplesRemaining--;
			if (rampSamplesRemaining == 0) {
				currentCarrierFrequency = targetCarrierFrequency;
				oscUpdateFrequency(&carrier, targetCarrierFrequency);
			}
		}

		const float ampEnvVal = adsrCalculateExp(&ampEnv);
		const float modEnvVal = adsrCalculateExp(&modEnv);

		if (ampEnv.state == IDLE) {
			state = VOICE_IDLE;
			break;
		}

		const float feedbackDepth = modulator.modIndex * modEnvVal;

		float perturbed = carrier.phase + (lastOutput * feedbackDepth * scalingConstant);

		if (perturbed < 0) perturbed += len;
		perturbed -= len * (float)((int)(perturbed * invLen));

		float currentSample = carrier.table[static_cast<int>(perturbed)];
		const float vol = ampEnvVal *  expresssion * masterVolume;
		lastOutput = currentSample;

		outputBuffer[i] += carrier.table[static_cast<int>(perturbed)] * vol;
		oscIncreasePhase(&carrier);
	}
}

void SynthVoice::processBlock(float* outputBuffer, size_t blockSize) {
	if (state == VOICE_IDLE) return;
	uint32_t cursor = 0;

	while (eventIndex < events.size()) {
		VoiceEvent& ev = events[eventIndex];
		if (type == STANDARD_PM) {
			renderInnerNormal(cursor, ev.offset, outputBuffer);
		} else if (type == FEEDBACK) {
			renderInnerFeedback(cursor, ev.offset, outputBuffer);
		}

		cursor = ev.offset;

		if (ev.type == EventType::NOTE_ON) {
			noteOn(ev.val, 0);
		} else if (ev.type == EventType::NOTE_OFF){
			noteOff();
		} else if (ev.type == EventType::PITCH_BEND) {
			setMidiBend(ev.val);
		} else if (ev.type == EventType::CC11_EXPRESSION) {
			expresssion = ev.val / 127.0f;
		} else if (ev.type == CC7_VOLUME) {
			masterVolume = ev.val / 127.0f;
		}

		eventIndex++;
	}

	if (type == STANDARD_PM) {
		renderInnerNormal(cursor, blockSize, outputBuffer);
	} else if (type == FEEDBACK) {
		renderInnerFeedback(cursor, blockSize, outputBuffer);
	}
	events.clear();
	eventIndex = 0;
}

void SynthVoice::pushEv(VoiceEvent& ev) {
  if (ev.type == NOTE_ON && state == VOICE_IDLE) {
    state = VOICE_WAIT;
  }

  events.push_back(ev);
}

void SynthVoice::setMidiBend(uint32_t bVal) {
	const float normalizedBend = (static_cast<float>(bVal) - 8192.0f) / 8192.0f;
	const float pitchBendSemitones = normalizedBend * pitchBendRange;

	float ratio = std::exp2(pitchBendSemitones / 12.0f);
    targetCarrierFrequency = baseCarrier * ratio;

	rampSamplesRemaining = 32;
	rampInc = (targetCarrierFrequency - currentCarrierFrequency) / 32;
}
