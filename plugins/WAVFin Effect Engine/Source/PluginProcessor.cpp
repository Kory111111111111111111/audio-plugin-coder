#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
WAVFinEffectEngineAudioProcessor::WAVFinEffectEngineAudioProcessor()
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize parameter pointers
    globalMixParam     = apvts.getRawParameterValue ("global_mix");
    outputGainParam    = apvts.getRawParameterValue ("output_gain");
    
    reverbEnableParam  = apvts.getRawParameterValue ("reverb_enable");
    delayEnableParam   = apvts.getRawParameterValue ("delay_enable");
    chorusEnableParam  = apvts.getRawParameterValue ("chorus_enable");
    filterEnableParam  = apvts.getRawParameterValue ("filter_enable");
    panEnableParam     = apvts.getRawParameterValue ("pan_enable");
    satEnableParam     = apvts.getRawParameterValue ("sat_enable");
    halftimeEnableParam = apvts.getRawParameterValue ("halftime_enable");
    vintageEnableParam  = apvts.getRawParameterValue ("vintage_enable");

    filterCutoffParam  = apvts.getRawParameterValue ("filter_cutoff");
    filterResParam     = apvts.getRawParameterValue ("filter_res");
    filterLfoRateParam = apvts.getRawParameterValue ("filter_lfo_rate");
    filterLfoDepthParam = apvts.getRawParameterValue ("filter_lfo_depth");
    
    satDriveParam      = apvts.getRawParameterValue ("sat_drive");
    satTypeParam       = apvts.getRawParameterValue ("sat_type");
    satMixParam        = apvts.getRawParameterValue ("sat_mix");

    chorusRateParam    = apvts.getRawParameterValue ("chorus_rate");
    chorusDepthParam   = apvts.getRawParameterValue ("chorus_depth");
    chorusMixParam     = apvts.getRawParameterValue ("chorus_mix");

    reverbSizeParam    = apvts.getRawParameterValue ("reverb_size");
    reverbDecayParam   = apvts.getRawParameterValue ("reverb_decay");
    reverbMixParam     = apvts.getRawParameterValue ("reverb_mix");

    delayTimeParam     = apvts.getRawParameterValue ("delay_time");
    delayFeedbackParam = apvts.getRawParameterValue ("delay_feedback");
    delayMixParam      = apvts.getRawParameterValue ("delay_mix");
    
    panRateParam       = apvts.getRawParameterValue ("pan_rate");
    panDepthParam      = apvts.getRawParameterValue ("pan_depth");
    
    halftimeMixParam   = apvts.getRawParameterValue ("halftime_mix");
    halftimeFadeParam  = apvts.getRawParameterValue ("halftime_fade");
    
    vintageWowParam    = apvts.getRawParameterValue ("vintage_wow");
    vintageFlutterParam = apvts.getRawParameterValue ("vintage_flutter");
    vintageNoiseParam  = apvts.getRawParameterValue ("vintage_noise");
}

WAVFinEffectEngineAudioProcessor::~WAVFinEffectEngineAudioProcessor()
{
}

//==============================================================================
const juce::String WAVFinEffectEngineAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WAVFinEffectEngineAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WAVFinEffectEngineAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WAVFinEffectEngineAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WAVFinEffectEngineAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WAVFinEffectEngineAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really using programs.
}

int WAVFinEffectEngineAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WAVFinEffectEngineAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const juce::String WAVFinEffectEngineAudioProcessor::getProgramName (int index)
{
    return {};
}

void WAVFinEffectEngineAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WAVFinEffectEngineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.prepare(spec);
    filter.setCutoffFrequency(20000.0f);  // Initialize to max frequency (no filtering)
    filter.setResonance(0.1f);             // Initialize to default resonance

    saturator.prepare(spec);
    saturator.functionToUse = [] (float x) { return std::tanh(x); };

    // Initialize Chorus with dry signal (no effect)
    chorus.prepare(spec);
    chorus.setRate(1.0f);
    chorus.setDepth(0.0f);     // No modulation depth
    chorus.setMix(0.0f);       // 100% dry signal
    
    // Initialize Reverb with dry signal (no effect)
    reverb.prepare(spec);
    juce::dsp::Reverb::Parameters revParams;
    revParams.roomSize = 0.5f;
    revParams.damping = 0.5f;
    revParams.wetLevel = 0.0f;  // 100% dry signal
    revParams.dryLevel = 1.0f;
    reverb.setParameters(revParams);
    
    // Initialize Output Gain to unity (0dB)
    outputGain.prepare(spec);
    outputGain.setRampDurationSeconds(0.02);
    outputGain.setGainDecibels(0.0f);  // Unity gain by default

    delayLine.prepare(spec);
    vintageDelay.prepare(spec);
    
    // Initialize smoothed delay time (50ms ramp to prevent clicks)
    smoothedDelayTime.reset(sampleRate, 0.05);  // 50ms ramp time
    smoothedDelayTime.setCurrentAndTargetValue(0.0f);
    
    // Initialize halftime buffer (approx 2.9 seconds at current sample rate for easy dual-voice wrap)
    // We want a power-of-two or a size that allows two voices at 180 deg phase
    int halftimeBufferSize = static_cast<int>(sampleRate * 2.0);
    halftimeBuffer.setSize(spec.numChannels, halftimeBufferSize);
    halftimeBuffer.clear();
    halftimeWritePos = 0;
    halftimeReadPos1 = 0.0f;
    halftimeReadPos2 = static_cast<float>(halftimeBufferSize / 2);
    halftimeBufferFilled = false;
    
    // Initialize global dry buffer
    globalDryBuffer.setSize(spec.numChannels, samplesPerBlock);
    globalDryBuffer.clear();
}

void WAVFinEffectEngineAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as a place to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WAVFinEffectEngineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some hosts (specifically those that use VST3) may not support mono.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void WAVFinEffectEngineAudioProcessor::updateParameters()
{
    if (filterCutoffParam && filterResParam)
    {
        filter.setCutoffFrequency (filterCutoffParam->load());
        filter.setResonance (filterResParam->load());
    }

    if (chorusRateParam && chorusDepthParam && chorusMixParam)
    {
        chorus.setRate (chorusRateParam->load());
        chorus.setDepth (chorusDepthParam->load() / 100.0f);
        chorus.setMix (chorusMixParam->load() / 100.0f);
    }

    if (reverbSizeParam && reverbMixParam)
    {
        juce::dsp::Reverb::Parameters revParams;
        revParams.roomSize = reverbSizeParam->load() / 100.0f;
        revParams.damping = 0.5f;
        revParams.wetLevel = reverbMixParam->load() / 100.0f;
        revParams.dryLevel = 1.0f - revParams.wetLevel;
        reverb.setParameters (revParams);
    }

    if (outputGainParam)
        outputGain.setGainDecibels (outputGainParam->load());
}

void WAVFinEffectEngineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateParameters();

    // Handle buffer size changes
    if (buffer.getNumSamples() != lastBufferSize)
    {
        lastBufferSize = buffer.getNumSamples();
        globalDryBuffer.setSize(buffer.getNumChannels(), lastBufferSize, false, false, true);
    }

    // 0. Capture dry signal for global mix
    if (globalMixParam && globalMixParam->load() < 99.0f)
    {
        globalDryBuffer.makeCopyOf(buffer);
    }

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);

    // 1. Halftime (FIXED: Frame-consistent state and dual-voice crossfading)
    if (halftimeEnableParam && halftimeEnableParam->load() > 0.5f)
    {
        float mix = halftimeMixParam->load() / 100.0f;
        float fade = halftimeFadeParam->load() / 100.0f; // Softness/Crossfade width
        int bufferSize = buffer.getNumSamples();
        int halftimeBufferSize = halftimeBuffer.getNumSamples();
        
        // We'll update positions once per process block for simplicity in the loop, 
        // but we'll track them carefully.
        for (int s = 0; s < bufferSize; ++s)
        {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* channelData = buffer.getWritePointer(ch);
                auto* halftimeData = halftimeBuffer.getWritePointer(ch);
                
                // Write current sample to circular buffer
                halftimeData[halftimeWritePos] = channelData[s];
                
                // Voice 1
                int r1_i = static_cast<int>(halftimeReadPos1);
                int r1_next = (r1_i + 1) % halftimeBufferSize;
                float f1 = halftimeReadPos1 - static_cast<float>(r1_i);
                float voice1 = halftimeData[r1_i] + f1 * (halftimeData[r1_next] - halftimeData[r1_i]);
                
                // Voice 2 (180 degrees out of phase)
                int r2_i = static_cast<int>(halftimeReadPos2);
                int r2_next = (r2_i + 1) % halftimeBufferSize;
                float f2 = halftimeReadPos2 - static_cast<float>(r2_i);
                float voice2 = halftimeData[r2_i] + f2 * (halftimeData[r2_next] - halftimeData[r2_i]);
                
                // Calculate crossfade gain based on distance between read and write heads
                float dist1 = static_cast<float>((halftimeWritePos - r1_i + halftimeBufferSize) % halftimeBufferSize);
                float gain1 = juce::jlimit(0.0f, 1.0f, dist1 / (halftimeBufferSize * 0.1f * (fade + 0.1f)));
                
                float dist2 = static_cast<float>((halftimeWritePos - r2_i + halftimeBufferSize) % halftimeBufferSize);
                float gain2 = 1.0f - gain1;
                
                float halftimeSample = (voice1 * gain1) + (voice2 * gain2);
                
                // Mix dry/wet
                channelData[s] = (halftimeSample * mix) + (channelData[s] * (1.0f - mix));
            }
            
            // Update positions ONCE per frame
            halftimeWritePos = (halftimeWritePos + 1) % halftimeBufferSize;
            halftimeReadPos1 += 0.5f;
            halftimeReadPos2 += 0.5f;
            
            if (halftimeReadPos1 >= static_cast<float>(halftimeBufferSize)) halftimeReadPos1 -= static_cast<float>(halftimeBufferSize);
            if (halftimeReadPos2 >= static_cast<float>(halftimeBufferSize)) halftimeReadPos2 -= static_cast<float>(halftimeBufferSize);
        }
    }

    // 2. Saturation (FIXED: Proper gain staging to prevent clipping)
    if (satEnableParam && satEnableParam->load() > 0.5f)
    {
        float driveDB = satDriveParam->load();
        float drive = juce::Decibels::decibelsToGain(driveDB);
        float mix = satMixParam->load() / 100.0f;
        
        // Properly allocate dry buffer (using temporary stack buffer for efficiency if small)
        juce::AudioBuffer<float> dryBuffer;
        dryBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, false);
        dryBuffer.makeCopyOf(buffer, true);

        // Apply drive and saturation with proper gain compensation
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* channelData = buffer.getWritePointer(ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                // Apply drive, saturate with tanh, then compensate gain
                float sample = channelData[s] * drive;
                sample = std::tanh(sample);  // Soft clipping
                sample /= (std::tanh(drive) + 0.0001f);  // Gain compensation to maintain level
                channelData[s] = sample;
            }
        }
        
        // Manual dry/wet blend
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* dry = dryBuffer.getReadPointer(ch);
            auto* wet = buffer.getWritePointer(ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
                wet[s] = (wet[s] * mix) + (dry[s] * (1.0f - mix));
        }
    }

    // 3. Filter with LFO modulation
    if (filterEnableParam && filterEnableParam->load() > 0.5f)
    {
        float lfoDepth = filterLfoDepthParam->load() / 100.0f;
        float lfoRate = filterLfoRateParam->load();
        
        if (lfoDepth > 0.01f)
        {
            // Apply LFO modulation to filter cutoff (block-rate for efficiency)
            float baseCutoff = filterCutoffParam->load();
            float lfoValue = std::sin(filterLfoPhase);
            float modulatedCutoff = baseCutoff * (1.0f + (lfoValue * lfoDepth));
            modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
            filter.setCutoffFrequency(modulatedCutoff);
            
            // Update LFO phase
            float lfoPhaseIncrement = (lfoRate * juce::MathConstants<float>::twoPi * buffer.getNumSamples()) / static_cast<float>(currentSampleRate);
            filterLfoPhase += lfoPhaseIncrement;
            if (filterLfoPhase >= juce::MathConstants<float>::twoPi)
                filterLfoPhase -= juce::MathConstants<float>::twoPi;
        }
        
        filter.process(context);
    }

    // 4. Vintage (FIXED: True pitch wow/flutter using delay line)
    if (vintageEnableParam && vintageEnableParam->load() > 0.5f)
    {
        float wowAmount = vintageWowParam->load() / 100.0f;
        float flutterAmount = vintageFlutterParam->load() / 100.0f;
        float noiseAmount = vintageNoiseParam->load() / 100.0f;
        
        // Characteristic tape speeds
        float wowFreq = 0.5f;     // 0.5 Hz for slow wow
        float flutterFreq = 8.0f;  // 8.0 Hz for fast flutter
        
        float baseDelayMs = 10.0f; // 10ms base delay
        float wowRangeMs = 2.0f * wowAmount;
        float flutterRangeMs = 0.5f * flutterAmount;
        
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            // Calculate current modulation value
            float wowMod = std::sin(vintageWowPhase) * wowRangeMs;
            float flutterMod = std::sin(vintageFlutterPhase) * flutterRangeMs;
            
            float totalDelayMs = baseDelayMs + wowMod + flutterMod;
            float delaySamples = (totalDelayMs / 1000.0f) * static_cast<float>(currentSampleRate);
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* channelData = buffer.getWritePointer(ch);
                float input = channelData[s];
                
                // Push to vintage delay line
                vintageDelay.pushSample(ch, input);
                
                // Pop with modulated delay time
                float modulated = vintageDelay.popSample(ch, delaySamples);
                
                // Add subtle tape hiss/noise if enabled
                if (noiseAmount > 0.01f)
                {
                    float noise = (randomGenerator.nextFloat() * 2.0f - 1.0f) * noiseAmount * 0.02f;
                    modulated += noise;
                }
                
                channelData[s] = modulated;
            }
            
            // Update phases
            vintageWowPhase += (wowFreq * juce::MathConstants<float>::twoPi) / static_cast<float>(currentSampleRate);
            vintageFlutterPhase += (flutterFreq * juce::MathConstants<float>::twoPi) / static_cast<float>(currentSampleRate);
            
            if (vintageWowPhase >= juce::MathConstants<float>::twoPi) vintageWowPhase -= juce::MathConstants<float>::twoPi;
            if (vintageFlutterPhase >= juce::MathConstants<float>::twoPi) vintageFlutterPhase -= juce::MathConstants<float>::twoPi;
        }
    }

    // 5. Chorus
    if (chorusEnableParam && chorusEnableParam->load() > 0.5f)
    {
        chorus.process (context);
    }

    // 6. Autopan
    if (panEnableParam && panEnableParam->load() > 0.5f)
    {
        float rate = panRateParam->load();
        float depth = panDepthParam->load() / 100.0f;
        float phaseIncrement = (rate * juce::MathConstants<float>::twoPi) / static_cast<float>(currentSampleRate);
        
        if (buffer.getNumChannels() >= 2)
        {
            auto* leftData = buffer.getWritePointer(0);
            auto* rightData = buffer.getWritePointer(1);
            
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float panValue = std::sin(panLfoPhase) * depth;
                float leftGain = 1.0f - ((panValue + 1.0f) * 0.5f * depth);
                float rightGain = 1.0f + ((panValue - 1.0f) * 0.5f * depth);
                
                leftData[s] *= leftGain;
                rightData[s] *= rightGain;
                
                panLfoPhase += phaseIncrement;
                if (panLfoPhase >= juce::MathConstants<float>::twoPi)
                    panLfoPhase -= juce::MathConstants<float>::twoPi;
            }
        }
    }

    // 7. Delay with feedback (OPTIMIZED: Smoothed delay time)
    if (delayEnableParam && delayEnableParam->load() > 0.5f)
    {
        float delayTimeMs = delayTimeParam->load();
        float feedback = delayFeedbackParam->load() / 100.0f;
        float mix = delayMixParam->load() / 100.0f;
        
        // Set target delay time (will be smoothed)
        float targetDelaySamples = (delayTimeMs / 1000.0f) * static_cast<float>(currentSampleRate);
        smoothedDelayTime.setTargetValue(targetDelaySamples);
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* channelData = buffer.getWritePointer(ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                // Get smoothed delay time (interpolated per-sample)
                float currentDelay = smoothedDelayTime.getNextValue();
                
                float input = channelData[s];
                float delayed = delayLine.popSample(ch, currentDelay);
                
                // Push input + feedback
                delayLine.pushSample(ch, input + (delayed * feedback));
                
                // Mix dry/wet
                channelData[s] = (delayed * mix) + (input * (1.0f - mix));
            }
        }
    }

    // 8. Reverb
    if (reverbEnableParam && reverbEnableParam->load() > 0.5f)
    {
        reverb.process (context);
    }

    // 9. Output Gain
    outputGain.process (context);

    // 10. Safety Soft Limiting (prevent clipping)
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float sample = channelData[s];
            // Soft limit above 0.9 to prevent hard clipping
            if (std::abs(sample) > 0.9f)
                sample = std::tanh(sample * 0.7f) / 0.7f;
            channelData[s] = sample;
        }
    }

    // 11. Global Mix (blend processed signal with original dry signal)
    if (globalMixParam)
    {
        float masterMix = globalMixParam->load() / 100.0f;
        if (masterMix < 0.99f)
        {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* wet = buffer.getWritePointer(ch);
                auto* dry = globalDryBuffer.getReadPointer(ch);
                for (int s = 0; s < buffer.getNumSamples(); ++s)
                    wet[s] = (wet[s] * masterMix) + (dry[s] * (1.0f - masterMix));
            }
        }
    }
}

//==============================================================================
bool WAVFinEffectEngineAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not have an editor)
}

juce::AudioProcessorEditor* WAVFinEffectEngineAudioProcessor::createEditor()
{
    return new WAVFinEffectEngineAudioProcessorEditor (*this);
}

//==============================================================================
void WAVFinEffectEngineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void WAVFinEffectEngineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout WAVFinEffectEngineAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto globalGroup = std::make_unique<juce::AudioProcessorParameterGroup>("global", "Global", "|");
    globalGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::global_mix, "Mix", 0.0f, 100.0f, 100.0f));
    globalGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::output_gain, "Output", -24.0f, 24.0f, 0.0f));
    layout.add(std::move(globalGroup));

    auto reverbGroup = std::make_unique<juce::AudioProcessorParameterGroup>("reverb", "Reverb", "|");
    reverbGroup->addChild(std::make_unique<juce::AudioParameterBool>(ParameterIDs::reverb_enable, "Enable", false));
    reverbGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::reverb_size, "Size", 0.0f, 100.0f, 50.0f));
    reverbGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::reverb_decay, "Decay", 0.1f, 10.0f, 2.0f));
    reverbGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::reverb_mix, "Mix", 0.0f, 100.0f, 30.0f));
    layout.add(std::move(reverbGroup));

    auto delayGroup = std::make_unique<juce::AudioProcessorParameterGroup>("delay", "Delay", "|");
    delayGroup->addChild(std::make_unique<juce::AudioParameterBool>(ParameterIDs::delay_enable, "Enable", false));
    delayGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::delay_time, "Time", 0.0f, 2000.0f, 500.0f));
    delayGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::delay_feedback, "Feedback", 0.0f, 100.0f, 40.0f));
    delayGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::delay_mix, "Mix", 0.0f, 100.0f, 30.0f));
    layout.add(std::move(delayGroup));

    auto chorusGroup = std::make_unique<juce::AudioProcessorParameterGroup>("chorus", "Chorus", "|");
    chorusGroup->addChild(std::make_unique<juce::AudioParameterBool>(ParameterIDs::chorus_enable, "Enable", false));
    chorusGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::chorus_rate, "Rate", 0.0f, 10.0f, 1.0f));
    chorusGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::chorus_depth, "Depth", 0.0f, 100.0f, 50.0f));
    chorusGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::chorus_mix, "Mix", 0.0f, 100.0f, 50.0f));
    layout.add(std::move(chorusGroup));

    auto filterGroup = std::make_unique<juce::AudioProcessorParameterGroup>("filter", "Filter", "|");
    filterGroup->addChild(std::make_unique<juce::AudioParameterBool>(ParameterIDs::filter_enable, "Enable", false));
    filterGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::filter_cutoff, "Cutoff", juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.3f), 2000.0f));
    filterGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::filter_res, "Resonance", 0.0f, 1.0f, 0.1f));
    filterGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::filter_lfo_rate, "LFO Rate", 0.0f, 20.0f, 2.0f));
    filterGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::filter_lfo_depth, "LFO Depth", 0.0f, 100.0f, 0.0f));
    layout.add(std::move(filterGroup));

    auto panGroup = std::make_unique<juce::AudioProcessorParameterGroup>("pan", "Autopan", "|");
    panGroup->addChild(std::make_unique<juce::AudioParameterBool>(ParameterIDs::pan_enable, "Enable", false));
    panGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::pan_rate, "Rate", 0.0f, 20.0f, 1.0f));
    panGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::pan_depth, "Depth", 0.0f, 100.0f, 100.0f));
    layout.add(std::move(panGroup));

    auto halftimeGroup = std::make_unique<juce::AudioProcessorParameterGroup>("halftime", "Halftime", "|");
    halftimeGroup->addChild(std::make_unique<juce::AudioParameterBool>(ParameterIDs::halftime_enable, "Enable", false));
    halftimeGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::halftime_mix, "Mix", 0.0f, 100.0f, 100.0f));
    halftimeGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::halftime_fade, "Smooth", 0.0f, 100.0f, 10.0f));
    layout.add(std::move(halftimeGroup));

    auto vintageGroup = std::make_unique<juce::AudioProcessorParameterGroup>("vintage", "Vintage", "|");
    vintageGroup->addChild(std::make_unique<juce::AudioParameterBool>(ParameterIDs::vintage_enable, "Enable", false));
    vintageGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::vintage_wow, "Wow", 0.0f, 100.0f, 20.0f));
    vintageGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::vintage_flutter, "Flutter", 0.0f, 100.0f, 20.0f));
    vintageGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::vintage_noise, "Noise", 0.0f, 100.0f, 0.0f));
    layout.add(std::move(vintageGroup));

    auto saturationGroup = std::make_unique<juce::AudioProcessorParameterGroup>("saturation", "Saturation", "|");
    saturationGroup->addChild(std::make_unique<juce::AudioParameterBool>(ParameterIDs::sat_enable, "Enable", false));
    saturationGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::sat_drive, "Drive", 0.0f, 48.0f, 0.0f));
    saturationGroup->addChild(std::make_unique<juce::AudioParameterChoice>(ParameterIDs::sat_type, "Type", juce::StringArray { "Tube", "Tape", "Diode", "Digital" }, 0));
    saturationGroup->addChild(std::make_unique<juce::AudioParameterFloat>(ParameterIDs::sat_mix, "Mix", 0.0f, 100.0f, 100.0f));
    layout.add(std::move(saturationGroup));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WAVFinEffectEngineAudioProcessor();
}
