//
// Created by adama on 5/7/26.
//
#include "dsp/wavetablegen.h"
#include <algorithm>
#include <vector>
#include "synth/VoiceManager.hpp"

constinit Program PRG_DEFAULT_BASS {
    0.001,
    1,
    {
        MS_TO_S(10),
        MS_TO_S(400),
        0.5,
        MS_TO_S(100)
    },
    {
        MS_TO_S(2),
        MS_TO_S(50),
        0.1,
        MS_TO_S(100)
    }
};

VoiceManager::VoiceManager(std::vector<TimedEvent> *timedEvents, float* modTable, float* carrierTable, float sr, size_t tableSize) {
    events.reserve(timedEvents->size());

    for (auto& v : voices) {
        v.init(&PRG_DEFAULT_BASS, modTable, carrierTable, sr, tableSize);
    }

    for (auto& e : *timedEvents) {
        const uint32_t blockId = e.timecode >> 6;
        const uint8_t offset = e.timecode & 63;

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

    std::stable_sort(events.begin(), events.end(), [](const VoiceEvent& a, const VoiceEvent& b) {
        return a.blockId < b.blockId;
    });
}
