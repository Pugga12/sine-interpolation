#include "MidiFile.h"
#include "MidiEvent.h"
#include "synth/VoiceManager.hpp"
#include <algorithm>
#include "synth/MidiPreprocessor.hpp"


bool MidiProcessor::load(const std::string &filename)
{
    if (!midiData.read(filename)) {
        return false;
    }

    midiData.doTimeAnalysis();

    midiData.linkNotePairs();

    midiData.joinTracks();

    processedEvents.reserve(midiData[0].size());

    return true;
}

void MidiProcessor::convert()
{
    for (int i = 0; i < midiData[0].size(); i++) {
        smf::MidiEvent& ev = midiData[0][i];

        if (ev.isNoteOn()) {
            smf::MidiEvent* offEvent = ev.getLinkedEvent();

            if (offEvent != nullptr) {
                uint32_t startTc = static_cast<uint32_t>(ev.seconds * 44100.0f);
                uint32_t endTc = static_cast<uint32_t>(offEvent->seconds * 44100.0f);

                uint8_t vNo = assignNoteToVoice(startTc, endTc);
                uint32_t pitch = static_cast<uint32_t>(ev.getP1());

                processedEvents.push_back(
                    {
                        vNo,
                        startTc,
                        NOTE_ON,
                        pitch
                    }
                );

                processedEvents.push_back(
                    {
                        vNo,
                        endTc,
                        NOTE_OFF,
                        pitch
                    }
                );
            }
        }
    }

    std::sort(processedEvents.begin(), processedEvents.end(), [](const TimedEvent& a, const TimedEvent& b) {
        if (a.timecode == b.timecode) {
            return a.type == NOTE_OFF;
        }

        return a.timecode < b.timecode;
    });
}

std::vector<TimedEvent> &MidiProcessor::getEvents()
{
    return processedEvents;
}

uint8_t MidiProcessor::assignNoteToVoice(uint32_t startTime, uint32_t endTime)
{
    for (int i = 0; i < voices.size(); i++) {
        auto& v = voices[i];

        if (v.endTime <= startTime) {
            v.startTime = startTime;
            v.endTime = endTime;
            return i;
        }
    }

    reassignments++;

    int victim = 0;
    uint32_t soonestEnd = voices[0].endTime;

    for (int i = 1; i < voices.size(); i++) {
        auto& v = voices[i];

        if (v.endTime < soonestEnd) {
            soonestEnd = v.endTime;
            victim = i;
        }
    }

    auto& victimVoice = voices[victim];
    
    victimVoice.startTime = startTime;
    victimVoice.endTime = endTime;
    
    return victim;
}
