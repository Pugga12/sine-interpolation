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
#include <cstdlib>
#include "synth/VoiceManager.hpp"
#include "synth/MidiPreprocessor.hpp"
#include "Options.h"

extern "C" {
	#include "dsp/wavetablegen.h"
    #include "io/wav.h"
}

constexpr size_t WAVETABLE_SIZE = 4096;
constexpr uint32_t SAMPLE_RATE = 44100;
constexpr float SAMPLE_RATE_F = static_cast<float>(SAMPLE_RATE);

int main(int argc, char** argv) {
    smf::Options options;
    options.process(argc, argv);
	std::vector<float> sineTbl(WAVETABLE_SIZE);
    
	wavetableGenSine(sineTbl.data(), WAVETABLE_SIZE);

    MidiProcessor mp;
    if (options.getArgCount() == 0) return -3;
    if(!mp.load(options.getArg(1))) {
        return -2;
    }
    mp.convert();

    size_t outputSize = mp.getFinalTc() + (SAMPLE_RATE / 4);
    std::vector<float> output(outputSize);
	VoiceManager vm(mp.getEvents(), sineTbl.data(), sineTbl.data(), SAMPLE_RATE_F, WAVETABLE_SIZE);

	if (!vm.go(output.data(), outputSize)) {
        std::printf("Invalid output size");
        return -1;
    }

    if (options.getArgCount() == 2) {
        auto& filename = options.getArg(2);
        writeWavF32(filename.c_str(), output.data(), outputSize, SAMPLE_RATE);
    } else {
        const std::string defaultName = "out.wav";
        writeWavF32(defaultName.c_str(), output.data(), outputSize, SAMPLE_RATE);
    }

	return 0;
}
