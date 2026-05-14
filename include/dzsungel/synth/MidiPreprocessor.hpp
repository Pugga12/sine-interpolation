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
#pragma once
#include "synth/SynthVoice.hpp"
#include "synth/VoiceManager.hpp"
#include "MidiFile.h"
#include <cstdint>
#include <vector>
#include <array>
using namespace smf;

struct PreprocessorVoiceState {
    uint32_t startTime = 0;
    uint32_t endTime = 0;
    uint32_t pitch = 0;
    uint32_t channel = 255;
};

struct ChannelState {
    uint16_t pitchBend = 0;
    uint32_t progId = 0;
    uint8_t expression = 127;
    uint8_t volume = 127;
    uint8_t pan = 64;
};

class MidiProcessor {
    private:
    std::vector<TimedEvent> processedEvents;
    std::array<PreprocessorVoiceState, 24> voices;
    std::array<ChannelState, 16> channelStates;
    std::array<ChannelState, 24> lastChannelStateUpdate;
    MidiFile midiData;
    size_t noteEvents = 0;
    size_t bendEvents = 0;
    std::string filename;
    std::array<std::vector<uint8_t>, 16> channelRosters;
    int reassignments = 0;
    int syncEvents = 0;
    size_t finalTc;

    uint8_t assignNoteToVoice(uint32_t startTime, uint32_t endTime, uint32_t pitch, uint32_t channel);
    void removeVoiceFromRosters(uint8_t voice);
    void printPreprocessorStats();
    void processNoteEvent(MidiEvent& stEv, MidiEvent* endEv);
    void processPitchBend(MidiEvent& ev);
    void processCc(MidiEvent& ev);
    void generateVoiceSetupEvents(uint8_t voice, uint32_t channel, uint32_t timecode);
    public:
    bool load(const std::string& filename);
    void convert();
    std::vector<TimedEvent>& getEvents();

    size_t getFinalTc() {
        return finalTc;
    }
};