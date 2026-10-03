cmake_minimum_required(VERSION 3.22)
project(S02 VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 9.0.3 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(S02Synth COMPANY_NAME qualitycoding PLUGIN_MANUFACTURER_CODE Qcod PLUGIN_CODE Cs02
  FORMATS VST3 PRODUCT_NAME "S02 Synth" IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(S02Synth PRIVATE Plugin.cpp)
target_compile_definitions(S02Synth PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(S02Synth PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
