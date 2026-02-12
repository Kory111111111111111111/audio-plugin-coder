cmake_minimum_required(VERSION 3.15)

project(WAVFinEffectEngine VERSION 0.0.0)

# Load JUCE
if (NOT TARGET juce::juce_audio_processors)
    find_package(JUCE CONFIG REQUIRED)
endif()

# Plugin definition
juce_add_plugin(WAVFinEffectEngine
    COMPANY_NAME "WAVFin Audio"
    BUNDLE_ID "com.WAVFinAudio.WAVFinEffectEngine"
    PLUGIN_MANUFACTURER_CODE "WvFn"
    PLUGIN_CODE "WfEe"
    FORMATS VST3 Standalone
    PRODUCT_NAME "WAVFin Effect Engine"
    NEEDS_WEBVIEW2 TRUE
)

# Embed web UI files into binary
# This creates a zip file and embeds it as C++ binary data
juce_add_binary_data(WAVFinEffectEngine_WebUI
    NAMESPACE WAVFinWebData
    SOURCES
        Source/ui/public/index.html
        Source/ui/public/js/index.js
        Source/ui/public/js/juce/juce_core.js
        Source/ui/public/js/juce/check_native_interop.js
)

# Source files
target_sources(WAVFinEffectEngine
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Include paths
target_include_directories(WAVFinEffectEngine
    PRIVATE
        Source
)

# Required JUCE modules
target_link_libraries(WAVFinEffectEngine
    PRIVATE
        WAVFinEffectEngine_WebUI
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# WebView2 compile definitions
target_compile_definitions(WAVFinEffectEngine
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
