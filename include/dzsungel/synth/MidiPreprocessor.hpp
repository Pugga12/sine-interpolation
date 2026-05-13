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
    std::array<PreprocessorVoiceState, 24> voices;
    smf::MidiFile midiData;
    std::string filename;
    int reassignments = 0;
    size_t finalTc;

    uint8_t assignNoteToVoice(uint32_t startTime, uint32_t endTime, uint32_t pitch);
    void printPreprocessorStats();
    public:
    bool load(const std::string& filename);
    void convert();
    std::vector<TimedEvent>& getEvents();
    size_t getFinalTc() {
        return finalTc;
    }
};