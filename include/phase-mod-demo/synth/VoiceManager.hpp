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
private:
	std::vector<VoiceEvent> events;
	std::array<SynthVoice, 12> voices;

	uint32_t maxBlock = 0;
	uint32_t maxEventTimecode = 0;
	float sr;

	void initPrintDbg();
public:
	VoiceManager(std::vector<TimedEvent>& timedEvents,float* modTable,float* carrierTable, float sr, size_t tableSize);

	bool go(float* outputBuffer, size_t outputSize);
};


