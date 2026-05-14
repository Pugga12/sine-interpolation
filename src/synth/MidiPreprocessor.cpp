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
#include "MidiFile.h"
#include "MidiEvent.h"
#include "synth/SynthVoice.hpp"
#include "synth/VoiceManager.hpp"
#include <algorithm>
#include "synth/MidiPreprocessor.hpp"
#include <iostream>
using namespace smf;

bool MidiProcessor::load(const std::string& filename)
{
    if (!midiData.read(filename)) {
        return false;
    }

    this->filename = filename;

    midiData.doTimeAnalysis();
    midiData.linkNotePairs();
    midiData.joinTracks();

    processedEvents.reserve(midiData[0].size());

    return true;
}

void MidiProcessor::convert()
{
    for (auto& r : channelRosters) {
        r.clear();
    }

    for (int i = 0; i < midiData[0].size(); i++) {
        MidiEvent& ev = midiData[0][i];

        if (ev.isNoteOn()) { 
            MidiEvent* offEvent = ev.getLinkedEvent();

            if (offEvent == nullptr) {
                continue;
            }

            processNoteEvent(ev, offEvent);
        } else if (ev.isPitchbend()) {
            processPitchBend(ev);
        } else if (ev.isController()) {
            processCc(ev);
        }
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

    auto& back = processedEvents.back();
    finalTc = back.timecode;

    printPreprocessorStats();
}

std::vector<TimedEvent>& MidiProcessor::getEvents()
{
    return processedEvents;
}

uint8_t MidiProcessor::assignNoteToVoice(uint32_t startTime, uint32_t endTime, uint32_t pitch, uint32_t channel)
{
    for (int i = 0; i < voices.size(); i++) {
        auto& voice = voices[i];

        if (voice.endTime + 1 < startTime) {
            removeVoiceFromRosters(i);

            voice.startTime = startTime;
            voice.endTime = endTime;
            voice.pitch = pitch;
            voice.channel = channel;
            channelRosters[channel].push_back(i);

            generateVoiceSetupEvents(i, channel, startTime);

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

    removeVoiceFromRosters(victim);

    voices[victim].startTime = startTime;
    voices[victim].endTime = endTime;
    voices[victim].pitch = pitch;
    voices[victim].channel = channel;

    channelRosters[channel].push_back(victim);
    generateVoiceSetupEvents(victim, channel, startTime);

    return victim;
}

void MidiProcessor::removeVoiceFromRosters(uint8_t voice)
{
    uint32_t vChannel = voices[voice].channel;
    
    if (vChannel != 255) {
        auto& cVec = channelRosters[vChannel];
        cVec.erase(std::remove(cVec.begin(), cVec.end(), voice), cVec.end());
    }
}

void MidiProcessor::printPreprocessorStats()
{
    std::cout << "=== MIDI File " << filename << " Preprocessing ===" << "\n";
    std::cout << "Total events in MIDI file: " << midiData[0].size() << "\n";
    std::cout << "Note Events Processed: " << noteEvents << "\n";
    std::cout << "Pitch Bends Processed: " << bendEvents << "\n";
    std::cout << "Total Output: " << processedEvents.size() << " event(s)" << "\n";
    std::cout << "Includes " << reassignments << " reassignment(s) and " << syncEvents << " voice syncs." << "\n";
}

void MidiProcessor::processNoteEvent(MidiEvent& ev, MidiEvent* offEvent) {
    uint32_t startTc = static_cast<uint32_t>(ev.seconds * 44100.0);
    uint32_t endTc = static_cast<uint32_t>(offEvent->seconds * 44100.0);
    uint32_t pitch = static_cast<uint32_t>(ev.getP1());
    uint32_t channel = static_cast<uint32_t>(ev.getChannel());

    uint8_t voice = assignNoteToVoice(startTc, endTc, pitch, channel);
    
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

    noteEvents += 2;
}

void MidiProcessor::processPitchBend(MidiEvent& ev) {
    uint32_t currentTc = static_cast<uint32_t>(ev.seconds * 44100.0);
    uint32_t channel = static_cast<uint32_t>(ev.getChannel());
    uint32_t bend = (static_cast<uint32_t>(ev.getP2() << 7) | static_cast<uint32_t>(ev.getP1()));

    auto& roster = channelRosters[channel];

    roster.erase(std::remove_if(roster.begin(), roster.end(), [&](uint8_t v) {
        return voices.at(v).endTime < currentTc;
    }), roster.end());

    for (uint8_t voiceId : roster) {
        processedEvents.push_back({
            voiceId,
            currentTc,
            PITCH_BEND,
            bend
        });

        lastChannelStateUpdate[voiceId].pitchBend = bend;
    }

    bendEvents++;
}

void MidiProcessor::processCc(MidiEvent& ev) {
    uint32_t ccNo = static_cast<uint32_t>(ev.getP1());

    if (ccNo > 127 || (ccNo != 7 && ccNo != 11)) return;

    uint32_t currentTc = static_cast<uint32_t>(ev.seconds * 44100.0f);
    uint32_t channel = static_cast<uint32_t>(ev.getChannel());
    uint32_t value = static_cast<uint32_t>(ev.getP2());

    auto& roster = channelRosters[channel];

    roster.erase(std::remove_if(roster.begin(), roster.end(), [&](uint8_t v) {
        return voices.at(v).endTime < currentTc;
    }), roster.end());


    switch (ccNo) {
        case 7: {
            channelStates[channel].volume = value;

            for (uint8_t voiceId : roster) {
                processedEvents.push_back({
                    voiceId,
                    currentTc,
                    CC7_VOLUME,
                    value
                });

                lastChannelStateUpdate[voiceId].volume = value;
            }

            break;
        }
        case 11: {
            channelStates[channel].expression = value;

            for (uint8_t voiceId : roster) {
                processedEvents.push_back({
                    voiceId,
                    currentTc,
                    CC11_EXPRESSION,
                    value
                });

                lastChannelStateUpdate[voiceId].expression = value;
            }

            break;
        }
        default: 
            break;
    }
}

void MidiProcessor::generateVoiceSetupEvents(uint8_t voice, uint32_t channel, uint32_t timecode) {
    if (lastChannelStateUpdate[voice].expression != channelStates[channel].expression) {
        uint8_t currentExpression = channelStates[channel].expression;
        lastChannelStateUpdate[voice].expression = currentExpression;

        processedEvents.push_back({
            voice,
            timecode,
            CC11_EXPRESSION,
            currentExpression
        });

        syncEvents++;
    }

    if (lastChannelStateUpdate[voice].pitchBend != channelStates[channel].pitchBend) {
        uint8_t currentPb = channelStates[channel].pitchBend;
        lastChannelStateUpdate[voice].pitchBend = currentPb;

        processedEvents.push_back({
            voice,
            timecode,
            PITCH_BEND,
            currentPb
        });

        syncEvents++;
    }

    if (lastChannelStateUpdate[voice].volume != channelStates[channel].volume) {
        uint8_t currentVol = channelStates[channel].volume;
        lastChannelStateUpdate[voice].volume = currentVol;

        processedEvents.push_back({
            voice,
            timecode,
            CC7_VOLUME,
            currentVol
        });
    }
}