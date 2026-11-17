cmake_minimum_required(VERSION 3.22)
project(MeuProQ VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)

# Baixa o JUCE automaticamente do GitHub
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com
    GIT_TAG 8.0.4
)
FetchContent_MakeAvailable(JUCE)

# Define o Plugin
juce_add_plugin(MeuProQ
    COMPANY_NAME "SuaMarca"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Mprq
    FORMATS VST3 Standalone
    PRODUCT_NAME "MeuProQ"
)

target_sources(MeuProQ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(MeuProQ PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MeuProQ PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
