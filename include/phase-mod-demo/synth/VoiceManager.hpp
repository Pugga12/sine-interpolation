//
// Created by adama on 5/7/26.
//
#pragma once
#include "synth/SynthVoice.hpp"
#include <array>

struct TimedEvent {
	uint8_t voiceId;
	uint32_t timecode;
	EventType type;
	uint32_t value;
};

class VoiceManager {
	std::vector<VoiceEvent> events;
	std::array<SynthVoice, 8> voices;

	uint32_t currentBlock = 0;
	uint32_t evCursor = 0;

	VoiceManager(std::vector<TimedEvent> *timedEvents,float* modTable,float* carrierTable, float sr, size_t tableSize);
};


