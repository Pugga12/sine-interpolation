cmake_minimum_required(VERSION 3.5)

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
    # FIXMATH_NO_ROUNDING
    # FIXMATH_OPTIMIZE_8BIT
)

# Include paths
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include/phase-mod-demo)

# Assign the include directories
include_directories(${INCLUDE_DIRS})

set(SOURCE_FILES src/sine.c src/wav.c)
add_executable(phase-mod-demo ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE m libfixmath)
