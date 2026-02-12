#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>
#include "ParameterIDs.h"

//==============================================================================
class WAVFinEffectEngineAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    WAVFinEffectEngineAudioProcessor();
    ~WAVFinEffectEngineAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // --- DSP Modules ---
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::WaveShaper<float> saturator;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;
    juce::dsp::Gain<float> outputGain;
    
    // Delay handling
    juce::dsp::DelayLine<float> delayLine { 192000 }; // Max 1s at 192kHz
    juce::SmoothedValue<float> smoothedDelayTime;
    
    // Vintage Delay for Wow/Flutter
    juce::dsp::DelayLine<float> vintageDelay { 4800 }; // Short delay for mod (approx 25ms at 192kHz)

    // LFO/Modulation state
    float filterLfoPhase = 0.0f;
    float panLfoPhase = 0.0f;
    double vintageWowPhase = 0.0;
    double vintageFlutterPhase = 0.0;
    
    // Halftime State
    double lastBarPosition = 0.0;
    int activeHalftimeVoice = 0; // 0 = Voice 1, 1 = Voice 2
    float halftimeCrossfade = 0.0f; // 0.0 = Voice 1 fully active, 1.0 = Voice 2 fully active
    
    // Halftime DSP
    juce::AudioBuffer<float> halftimeBuffer;
    int halftimeWritePos = 0;
    float halftimeReadPos1 = 0.0f;
    float halftimeReadPos2 = 0.0f;
    bool halftimeBufferFilled = false;
    
    // Global mix dry buffer
    juce::AudioBuffer<float> globalDryBuffer;
    double currentSampleRate = 44100.0;
    int lastBufferSize = 0;
    
    // Thread-safe random number generator for vintage noise
    juce::Random randomGenerator;

    // Parameter References (for performance)
    std::atomic<float>* globalMixParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
    
    // Module Enables
    std::atomic<float>* reverbEnableParam = nullptr;
    std::atomic<float>* delayEnableParam = nullptr;
    std::atomic<float>* chorusEnableParam = nullptr;
    std::atomic<float>* filterEnableParam = nullptr;
    std::atomic<float>* panEnableParam = nullptr;
    std::atomic<float>* satEnableParam = nullptr;
    std::atomic<float>* halftimeEnableParam = nullptr;
    std::atomic<float>* vintageEnableParam = nullptr;

    // Module Params
    std::atomic<float>* filterCutoffParam = nullptr;
    std::atomic<float>* filterResParam = nullptr;
    std::atomic<float>* filterLfoRateParam = nullptr;
    std::atomic<float>* filterLfoDepthParam = nullptr;
    
    std::atomic<float>* satDriveParam = nullptr;
    std::atomic<float>* satTypeParam = nullptr;
    std::atomic<float>* satMixParam = nullptr;
    
    std::atomic<float>* chorusRateParam = nullptr;
    std::atomic<float>* chorusDepthParam = nullptr;
    std::atomic<float>* chorusMixParam = nullptr;
    
    std::atomic<float>* reverbSizeParam = nullptr;
    std::atomic<float>* reverbDecayParam = nullptr;
    std::atomic<float>* reverbMixParam = nullptr;

    std::atomic<float>* delayTimeParam = nullptr;
    std::atomic<float>* delayFeedbackParam = nullptr;
    std::atomic<float>* delayMixParam = nullptr;
    
    std::atomic<float>* panRateParam = nullptr;
    std::atomic<float>* panDepthParam = nullptr;
    
    std::atomic<float>* halftimeMixParam = nullptr;
    std::atomic<float>* halftimeFadeParam = nullptr;
    
    std::atomic<float>* vintageWowParam = nullptr;
    std::atomic<float>* vintageFlutterParam = nullptr;
    std::atomic<float>* vintageNoiseParam = nullptr;
    
    void updateParameters();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WAVFinEffectEngineAudioProcessor)
};
