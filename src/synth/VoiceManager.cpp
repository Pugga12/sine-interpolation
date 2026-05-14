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
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <vector>
#include "synth/VoiceManager.hpp"
#include "synth/SynthVoice.hpp"

constinit Program PRG_DEFAULT_BASS {
    2,
    0.0001,
    {
        MS_TO_S(10),
        MS_TO_S(400),
        0.5,
        MS_TO_S(500)
    },
    {
        MS_TO_S(10),
        MS_TO_S(50),
        0.1,
        MS_TO_S(500)
    },
    FEEDBACK
};

void VoiceManager::initPrintDbg() {
    std::printf("=== Voice Manager Debug ===\n");
    std::printf("Events Processed: %lu\n", events.size());
    std::printf("Max Event Timecode: %u\n", maxEventTimecode);
    std::printf("Max Block ID: %u\n", maxBlock);
    std::printf("Fractional Blocks / Full Blocks / Remainder Block Size: %f, %u, %u\n", maxEventTimecode / 64.0f, maxEventTimecode / 64, maxEventTimecode % 64);
}

VoiceManager::VoiceManager(std::vector<TimedEvent>& timedEvents, float* modTable, float* carrierTable, float sr, size_t tableSize) : sr(sr) {
    events.reserve(timedEvents.size());

    for (auto& v : voices) {
        v.init(&PRG_DEFAULT_BASS, modTable, carrierTable, sr, tableSize);
    }

    uint32_t maxBlock = 0;
    uint32_t maxEventTimecode = 0;

    for (auto& e : timedEvents) {
        const uint32_t blockId = e.timecode >> 6;
        const uint8_t offset = e.timecode & 63;

        if (blockId > maxBlock) {
            maxBlock = blockId;
        }

        if (e.timecode > maxEventTimecode) {
            maxEventTimecode = e.timecode;
        }

        events.push_back(
            {
                blockId,
                e.voiceId,
                e.type,
                offset,
                e.value
            }
        );
    }

    std::sort(events.begin(), events.end(), [](const VoiceEvent& a, const VoiceEvent& b) {
        if (a.blockId != b.blockId) {
            return a.blockId < b.blockId;
        }
        return a.offset < b.offset; // Secondary sort key
    });

    this->maxBlock = maxBlock;
    this->maxEventTimecode = maxEventTimecode;

    initPrintDbg();
}

bool VoiceManager::go(float* outputBuffer, size_t outputSize) {
    uint32_t remBlockSize = outputSize % 64;

    if (maxEventTimecode > outputSize) {
        return false;
    }

    uint32_t fullBlocks = outputSize / 64;
    uint32_t currentEvent = 0;
    float* bSt = outputBuffer;

    for (uint32_t i = 0; i < fullBlocks; i++) {
        bSt = outputBuffer + (64 * i); 

        while (currentEvent < events.size() && events[currentEvent].blockId == i) {
            VoiceEvent& ev = events[currentEvent];

            if (ev.voiceId < voices.size()) {
                voices[ev.voiceId].pushEv(ev);
            }
            currentEvent++;
        }

         for (auto& v : voices) {
            if (v.getState() != VOICE_IDLE) {
                 v.processBlock(bSt, 64);
            }
        }
    }

    if (remBlockSize > 0) {
        uint32_t fractionalBlockId = fullBlocks;
        float* bSt = outputBuffer + (64 * fullBlocks);

        while (currentEvent < events.size() && events[currentEvent].blockId == fractionalBlockId) {
            VoiceEvent& ev = events[currentEvent];
            if (ev.voiceId < voices.size()) {
                voices[ev.voiceId].pushEv(ev);
            }
            currentEvent++;
        }

        for (auto& v : voices) {
            if (v.getState() != VOICE_IDLE) {
                v.processBlock(bSt, remBlockSize);
            }
        }
    }

    for (int i = 0; i < outputSize; i++) {
        outputBuffer[i] = std::tanh(outputBuffer[i] * 0.5f);
    }

    return true;
}
