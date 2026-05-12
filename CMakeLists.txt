cmake_minimum_required(VERSION 3.10)

# Definitions
set(PROJECT_NAME phase-mod-demo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#project(cmake_c_template)
project(${PROJECT_NAME})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include(FetchContent)
FetchContent_Declare(
    libfixmath
    GIT_REPOSITORY https://github.com/PetteriAimonen/libfixmath.git
    GIT_TAG 9318ccd9428114d318eb8703a7e40c152b9059c7
)
FetchContent_MakeAvailable(libfixmath)

target_compile_definitions(libfixmath PRIVATE
    # FIXMATH_FAST_SIN
    # FIXMATH_NO_64BIT
    # FIXMATH_NO_CACHE
    # FIXMATH_NO_HARD_DIVISION
    # FIXMATH_NO_OVERFLOW
    FIXMATH_NO_ROUNDING
    # FIXMATH_OPTIMIZE_8BIT
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include paths
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include/phase-mod-demo)

# Assign the include directories
include_directories(${INCLUDE_DIRS})
option(EXPONENTIAL_ADSR "Use exponential/log adsr calculations" ON)

set(MIDIFILE_SOURCES
    src/midifile/Options.cpp
    src/midifile/Binasc.cpp
    src/midifile/MidiEvent.cpp
    src/midifile/MidiEventList.cpp
    src/midifile/MidiFile.cpp
    src/midifile/MidiMessage.cpp
)

set(COMMON_SOURCES src/io/wav.c src/dsp/adsr.c src/dsp/oscillator.c
        src/dsp/noise.cpp
        src/synth/SynthVoice.cpp
        src/synth/VoiceManager.cpp
        src/dsp/wavetablegen.c
        src/synth/MidiPreprocessor.cpp
)

set(MAIN_FILE src/new-main.cpp)
add_executable(phase-mod-demo ${COMMON_SOURCES} ${MAIN_FILE} ${MIDIFILE_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE m libfixmath)
target_include_directories(phase-mod-demo PRIVATE ${libfixmath_SOURCE_DIR}/libfixmath include/midifile)
if(EXPONENTIAL_ADSR)
    target_compile_definitions(phase-mod-demo PRIVATE EXPONENTIAL_ADSR)
endif ()
