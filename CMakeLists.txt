cmake_minimum_required(VERSION 3.15)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum OS X deployment version" FORCE)

project(Repitch VERSION 0.4.0)

include(FetchContent)
FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 6.1.5
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Repitch
    ICON_BIG "icon.png"
    COMPANY_NAME "Max Pollack"
    COMPANY_WEBSITE "maxis.cool"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Repitch requires an audio input"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE MaxP
    DESCRIPTION "a MIDI-controlled harmonizer"
    PLUGIN_CODE Repi
    FORMATS AU VST3 Standalone
    VST3_CATEGORIES Pitch\ Shift
    AU_MAIN_TYPE kAudioUnitType_MusicEffect
    PRODUCT_NAME "Repitch")

juce_generate_juce_header(Repitch)

target_sources(Repitch PRIVATE
    PluginEditor.cpp
    PluginProcessor.cpp)

target_compile_definitions(Repitch
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Repitch PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp)

juce_disable_default_flags()
set(CMAKE_C_FLAGS_RELEASE, "-O3 -ffast-math")
