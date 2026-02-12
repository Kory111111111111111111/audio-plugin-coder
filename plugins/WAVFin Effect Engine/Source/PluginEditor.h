#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "ParameterIDs.h"

//==============================================================================
class WAVFinEffectEngineAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    WAVFinEffectEngineAudioProcessorEditor (WAVFinEffectEngineAudioProcessor&);
    ~WAVFinEffectEngineAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WAVFinEffectEngineAudioProcessor& audioProcessor;

    // ═══════════════════════════════════════════════════════════════════
    // CRITICAL: Member Declaration Order (Prevents DAW Crashes)
    // ═══════════════════════════════════════════════════════════════════
    // 1. PARAMETER RELAYS FIRST
    
    // Global
    juce::WebSliderRelay globalMixRelay       { "global_mix" };
    juce::WebSliderRelay outputGainRelay      { "output_gain" };

    // Reverb
    juce::WebToggleButtonRelay  reverbEnableRelay   { "reverb_enable" };
    juce::WebSliderRelay reverbSizeRelay     { "reverb_size" };
    juce::WebSliderRelay reverbDecayRelay    { "reverb_decay" };
    juce::WebSliderRelay reverbMixRelay      { "reverb_mix" };

    // Delay
    juce::WebToggleButtonRelay  delayEnableRelay    { "delay_enable" };
    juce::WebSliderRelay delayTimeRelay      { "delay_time" };
    juce::WebSliderRelay delayFeedbackRelay  { "delay_feedback" };
    juce::WebSliderRelay delayMixRelay       { "delay_mix" };

    // Chorus
    juce::WebToggleButtonRelay  chorusEnableRelay   { "chorus_enable" };
    juce::WebSliderRelay chorusRateRelay     { "chorus_rate" };
    juce::WebSliderRelay chorusDepthRelay    { "chorus_depth" };
    juce::WebSliderRelay chorusMixRelay      { "chorus_mix" };

    // AutoFilter
    juce::WebToggleButtonRelay  filterEnableRelay   { "filter_enable" };
    juce::WebSliderRelay filterCutoffRelay   { "filter_cutoff" };
    juce::WebSliderRelay filterResRelay      { "filter_res" };
    juce::WebSliderRelay filterLfoRateRelay  { "filter_lfo_rate" };
    juce::WebSliderRelay filterLfoDepthRelay { "filter_lfo_depth" };

    // Autopan
    juce::WebToggleButtonRelay  panEnableRelay      { "pan_enable" };
    juce::WebSliderRelay panRateRelay        { "pan_rate" };
    juce::WebSliderRelay panDepthRelay       { "pan_depth" };

    // Halftime
    juce::WebToggleButtonRelay  halftimeEnableRelay { "halftime_enable" };
    juce::WebSliderRelay halftimeMixRelay    { "halftime_mix" };
    juce::WebSliderRelay halftimeFadeRelay   { "halftime_fade" };

    // Vintage
    juce::WebToggleButtonRelay  vintageEnableRelay  { "vintage_enable" };
    juce::WebSliderRelay vintageWowRelay     { "vintage_wow" };
    juce::WebSliderRelay vintageFlutterRelay { "vintage_flutter" };
    juce::WebSliderRelay vintageNoiseRelay   { "vintage_noise" };

    // Saturation
    juce::WebToggleButtonRelay  satEnableRelay      { "sat_enable" };
    juce::WebSliderRelay satDriveRelay       { "sat_drive" };
    juce::WebComboBoxRelay satTypeRelay      { "sat_type" };
    juce::WebSliderRelay satMixRelay         { "sat_mix" };


    // 2. WEBVIEW SECOND
    std::unique_ptr<juce::WebBrowserComponent> webView;


    // 3. PARAMETER ATTACHMENTS LAST
    std::unique_ptr<juce::WebSliderParameterAttachment> globalMixAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> outputGainAttachment;

    std::unique_ptr<juce::WebToggleButtonParameterAttachment> reverbEnableAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> reverbSizeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> reverbDecayAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> reverbMixAttachment;

    std::unique_ptr<juce::WebToggleButtonParameterAttachment> delayEnableAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> delayTimeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> delayFeedbackAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> delayMixAttachment;

    std::unique_ptr<juce::WebToggleButtonParameterAttachment> chorusEnableAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> chorusRateAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> chorusDepthAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> chorusMixAttachment;

    std::unique_ptr<juce::WebToggleButtonParameterAttachment> filterEnableAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> filterCutoffAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> filterResAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> filterLfoRateAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> filterLfoDepthAttachment;

    std::unique_ptr<juce::WebToggleButtonParameterAttachment> panEnableAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> panRateAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> panDepthAttachment;

    std::unique_ptr<juce::WebToggleButtonParameterAttachment> halftimeEnableAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> halftimeMixAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> halftimeFadeAttachment;

    std::unique_ptr<juce::WebToggleButtonParameterAttachment> vintageEnableAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> vintageWowAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> vintageFlutterAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> vintageNoiseAttachment;

    std::unique_ptr<juce::WebToggleButtonParameterAttachment> satEnableAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> satDriveAttachment;
    std::unique_ptr<juce::WebComboBoxParameterAttachment> satTypeAttachment;
    std::unique_ptr<juce::WebSliderParameterAttachment> satMixAttachment;


    // Resource provider function
    std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url);

    // Helper functions for resource provider
    static const char* getMimeForExtension (const juce::String& extension);
    static juce::String getExtension (juce::String filename);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WAVFinEffectEngineAudioProcessorEditor)
};
