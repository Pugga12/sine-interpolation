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
#include <vector>

extern "C" {
#include "dsp/oscillator.h"
#include "dsp/adsr.h"
}

enum VoiceState {
    VOICE_IDLE,
    VOICE_WAIT,
    VOICE_ACTIVE,
    VOICE_RELEASING,
};

enum EventType {
    // CC event (0 - 127)
    CC7_VOLUME = 7,
    CC11_EXPRESSION = 11,

    // other events (>127)
    NOTE_ON = 128,
    NOTE_OFF = 129,
    PROGRAM_CHANGE = 130,
    PITCH_BEND = 131,
};

enum OscillatorType {
    STANDARD_PM,
    FEEDBACK
};

struct Program {
    float modIndex;
    float cToMRatio;
    Envelope ampEnv;
    Envelope modEnv;
    OscillatorType type;
};

struct VoiceEvent {
    uint32_t blockId;
    uint8_t voiceId;
    EventType type;

    uint8_t offset;
    uint32_t val;
};

class SynthVoice {
    private:
        std::vector<VoiceEvent> events;
        OscillatorType type;
        Oscillator carrier;
        Oscillator modulator;
        ADSR ampEnv;
        ADSR modEnv;

        uint32_t currentMidiNote = 0;
        uint8_t eventIndex = 0;
        VoiceState state = VOICE_IDLE;
        float sampleRate;
        float cToMRatio;
        
        float pitchBendRange = 2.0f;
        float baseCarrier = 0.0f;
        float currentCarrierFrequency = 0.0f;
        float targetCarrierFrequency = 0.0f;
        float rampInc = 0.0f;
        uint32_t rampSamplesRemaining = 0;

        float expresssion = 1.0f;
        float masterVolume = 1.0f;
        
        float lastOutput = 0;

        void renderInnerNormal(uint32_t start, uint32_t end, float* outputBuffer);
        void renderInnerFeedback(uint32_t start, uint32_t end, float* output);
        void setMidiBend(uint32_t bVal);
    public:
        void noteOn(uint32_t midiNote);
        void noteOff();
        void processBlock(float* outputBuffer, size_t blockSize);
        void pushEv(VoiceEvent& ev);
        void init(Program* program, float* modTable, float* carrierTable, float sr, size_t tableSize);
        
        VoiceState getState() {
            return state;
        }

        uint32_t getNote() {
            return currentMidiNote;
        }

        // default constructor, to make the VoiceManager init straight-forward
        SynthVoice() :
            carrier{},
            modulator{},
            ampEnv{},
            modEnv{},
            sampleRate(44100),
            cToMRatio(1),
            type(STANDARD_PM)
        {
            events.reserve(8);
        }
};

