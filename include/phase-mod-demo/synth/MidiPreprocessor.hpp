#pragma once
#include "synth/SynthVoice.hpp"
#include "synth/VoiceManager.hpp"
#include "MidiFile.h"
#include <vector>
#include <array>

struct PreprocessorVoiceState {
    uint32_t startTime = 0;
    uint32_t endTime = 0;
    uint32_t pitch = 0;
};

class MidiProcessor {
    private:
    std::vector<TimedEvent> processedEvents;
    std::array<PreprocessorVoiceState, 12> voices;
    smf::MidiFile midiData;
    int reassignments = 0;

    uint8_t assignNoteToVoice(uint32_t startTime, uint32_t endTime, uint32_t pitch);
    public:
    bool load(const std::string& filename);
    void convert();
    std::vector<TimedEvent>& getEvents();
};