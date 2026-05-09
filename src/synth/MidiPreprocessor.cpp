#include "MidiFile.h"
#include "MidiEvent.h"
#include "synth/VoiceManager.hpp"
#include <algorithm>
#include "synth/MidiPreprocessor.hpp"

bool MidiProcessor::load(const std::string& filename)
{
    if (!midiData.read(filename)) {
        return false;
    }

    midiData.doTimeAnalysis();
    midiData.linkNotePairs();
    midiData.joinTracks();

    processedEvents.reserve(midiData[0].size() * 2);

    return true;
}

void MidiProcessor::convert()
{
    for (int i = 0; i < midiData[0].size(); i++) {
        smf::MidiEvent& ev = midiData[0][i];

        if (!ev.isNoteOn()) {
            continue;
        }

        smf::MidiEvent* offEvent = ev.getLinkedEvent();

        if (offEvent == nullptr) {
            continue;
        }

        uint32_t startTc = static_cast<uint32_t>(ev.seconds * 44100.0);
        uint32_t endTc = static_cast<uint32_t>(offEvent->seconds * 44100.0);
        uint32_t pitch = static_cast<uint32_t>(ev.getP1());
        uint8_t voice = assignNoteToVoice(startTc, endTc, pitch);

        processedEvents.push_back({
            voice,
            startTc,
            NOTE_ON,
            pitch
        });

        processedEvents.push_back({
            voice,
            endTc,
            NOTE_OFF,
            pitch
        });
    }

    std::sort(
        processedEvents.begin(),
        processedEvents.end(),
        [](const TimedEvent& a, const TimedEvent& b)
        {
            if (a.timecode != b.timecode) {
                return a.timecode < b.timecode;
            }

            if (a.type != b.type) {
                return a.type == NOTE_ON;
            }

            return a.voiceId < b.voiceId;
        }
    );
}

std::vector<TimedEvent>& MidiProcessor::getEvents()
{
    return processedEvents;
}

uint8_t MidiProcessor::assignNoteToVoice(uint32_t startTime, uint32_t endTime, uint32_t pitch)
{
    for (int i = 0; i < voices.size(); i++) {
        auto& voice = voices[i];

        if (voice.endTime + 1 < startTime) {
            voice.startTime = startTime;
            voice.endTime = endTime;
            voice.pitch = pitch;

            return i;
        }
    }

    reassignments++;
    int victim = 0;
    uint32_t soonestEnd = voices[0].endTime;

    for (int i = 1; i < voices.size(); i++) {
        if (voices[i].endTime < soonestEnd) {
            soonestEnd = voices[i].endTime;
            victim = i;
        }
    }

    processedEvents.push_back({
        (uint8_t)victim,
        startTime,
        NOTE_OFF,
        voices[victim].pitch
    });

    voices[victim].startTime = startTime;
    voices[victim].endTime = endTime;
    voices[victim].pitch = pitch;

    return victim;
}