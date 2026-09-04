cmake_minimum_required(VERSION 3.25)
project(SCREAMBACK VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED IPLUG2_DIR)
  set(IPLUG2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../iPlug2" CACHE PATH "iPlug2 root directory")
endif()

if(NOT EXISTS "${IPLUG2_DIR}/iPlug2.cmake")
  message(FATAL_ERROR "iPlug2 not found at ${IPLUG2_DIR}. Clone https://github.com/iPlug2/iPlug2 next to this repository or pass -DIPLUG2_DIR=/path/to/iPlug2")
endif()

include(${IPLUG2_DIR}/iPlug2.cmake)
find_package(iPlug2 REQUIRED)

set(SCREAMBACK_FORMATS VST3)
if(APPLE)
  list(APPEND SCREAMBACK_FORMATS AU)
endif()

iplug_add_plugin(${PROJECT_NAME}
  SOURCES
    SCREAMBACK.cpp
    SCREAMBACK.h
    DSP/PitchDetector.h
    DSP/FeedbackEngine.h
    resources/resource.h
  RESOURCES
    ${IPLUG2_DIR}/Examples/IPlugEffect/resources/fonts/Roboto-Regular.ttf
  FORMATS
    ${SCREAMBACK_FORMATS}
)

option(SCREAMBACK_BUILD_TESTS "Build lightweight DSP smoke tests" ON)
if(SCREAMBACK_BUILD_TESTS)
  enable_testing()
  add_executable(SCREAMBACK-dsp-smoke tests/dsp_smoke.cpp)
  target_include_directories(SCREAMBACK-dsp-smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME screamback_dsp_smoke COMMAND SCREAMBACK-dsp-smoke)
endif()
