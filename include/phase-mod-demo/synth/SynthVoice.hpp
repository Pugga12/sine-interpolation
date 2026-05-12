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
    NOTE_ON,
    NOTE_OFF,
    PROGRAM_CHANGE
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
        
        float lastOutput = 0;

        void renderInnerNormal(uint32_t start, uint32_t end, float* outputBuffer);
        void renderInnerFeedback(uint32_t start, uint32_t end, float* output);
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

