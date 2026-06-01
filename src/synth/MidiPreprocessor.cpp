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
#include "synth/VoiceManager.hpp"
#include <algorithm>
#include "synth/MidiPreprocessor.hpp"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "types.hpp"
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
        } else if (ev.isPatchChange()) {
            processProgramChange(ev);
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
    auto& cVec = channelRosters[channel];
    for (int i = 0; i < cVec.size(); i++) {
        uint8_t vId = cVec[i];
        auto& voice = voices[vId];

        if (voice.endTime + 1 < startTime) {
            voice.startTime = startTime;
            voice.endTime = endTime;
            voice.pitch = pitch;
            voice.channel = channel;

            generateVoiceSetupEvents(vId, channel, startTime);

            return vId;
        }
    }

    uint8_t newVoice = 0;
    try {
        newVoice = availableVoices.at(0);
        availableVoices.erase(availableVoices.begin());
    } catch (const std::out_of_range& e) {
        std::cerr << "Ran out of voices while trying to assign to channel " << channel << ". Please increase the MAX_VOICES option." << "\n";
        if (channelRosters[channel].size() == 0) {
            std::string msg = "Attempted to reassign a voice on channel " + std::to_string(channel) + " but no candidates were found.";
            throw std::runtime_error(msg);
        }
        return stealVoiceInChannel(startTime, endTime, pitch, channel);
        
    }

    cVec.push_back(newVoice);

    voices[newVoice].startTime = startTime;
    voices[newVoice].endTime = endTime;
    voices[newVoice].pitch = pitch;
    voices[newVoice].channel = channel;
    generateVoiceSetupEvents(newVoice, channel, startTime);

    return newVoice;
}

uint8_t MidiProcessor::stealVoiceInChannel(uint32_t startTime, uint32_t endTime, uint32_t pitch, uint32_t channel) {
    reassignments++;
    int victim = 0;
    auto& cVec = channelRosters[channel];
    uint32_t soonestEnd = voices[cVec[0]].endTime;

    for (int i = 1; i < cVec.size(); i++) {
        uint8_t vId = cVec[i];
        auto& v = voices[vId];
        if (v.endTime < soonestEnd) {
            soonestEnd = v.endTime;
            victim = vId;
        }
    }

    processedEvents.push_back({
        (uint8_t)victim,
        startTime,
        NOTE_OFF,
        0
    });

    voices[victim].startTime = startTime;
    voices[victim].endTime = endTime;
    voices[victim].pitch = pitch;

    generateVoiceSetupEvents(victim, channel, startTime);

    return victim;
}

void MidiProcessor::printPreprocessorStats()
{
    std::cout << "=== MIDI File " << filename << " Preprocessing ===" << "\n";
    std::cout << "Total events in MIDI file: " << midiData[0].size() << "\n";
    std::cout << "Note Events Processed: " << noteEvents << "\n";
    std::cout << "Pitch Bends Processed: " << bendEvents << "\n";
    std::cout << "Total Output: " << processedEvents.size() << " event(s)" << "\n";
    std::cout << "Includes " << reassignments << " reassignment(s) and " << syncEvents << " voice syncs." << "\n";
    std::cout << "Voices free at end of preprocessing: " << availableVoices.size() << "\n";
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
        
    std::vector<uint8_t>& roster = channelRosters[channel];

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

    std::vector<uint8_t>& roster = channelRosters[channel];

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

void MidiProcessor::processProgramChange(MidiEvent& ev) {
    uint32_t currentTc = static_cast<uint32_t>(ev.seconds * 44100.0f);
    uint32_t channel = static_cast<uint32_t>(ev.getChannel());
    uint8_t value = static_cast<uint8_t>(ev.getP1());
    
    std::vector<uint8_t>& roster = channelRosters[channel];
    channelStates[channel].programId = value;

    for (uint8_t voiceId : roster) {
        processedEvents.push_back({
            voiceId,
            currentTc,
            PROGRAM_CHANGE,
            value
        });
        
        lastChannelStateUpdate[voiceId].programId = value;
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

        syncEvents++;
    }

    if (lastChannelStateUpdate[voice].programId != channelStates[channel].programId) {
        uint8_t currentProgram = channelStates[channel].programId;
        lastChannelStateUpdate[voice].programId = currentProgram;

        processedEvents.push_back({
            voice,
            timecode,
            PROGRAM_CHANGE,
            currentProgram
        });

        syncEvents++;
    }
    
}
