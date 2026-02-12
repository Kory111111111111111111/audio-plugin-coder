#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParameterIDs
{
    inline const juce::ParameterID global_mix       { "global_mix", 1 };
    inline const juce::ParameterID output_gain     { "output_gain", 1 };

    // Reverb
    inline const juce::ParameterID reverb_enable    { "reverb_enable", 1 };
    inline const juce::ParameterID reverb_size      { "reverb_size", 1 };
    inline const juce::ParameterID reverb_decay     { "reverb_decay", 1 };
    inline const juce::ParameterID reverb_mix       { "reverb_mix", 1 };

    // Delay
    inline const juce::ParameterID delay_enable     { "delay_enable", 1 };
    inline const juce::ParameterID delay_time       { "delay_time", 1 };
    inline const juce::ParameterID delay_feedback   { "delay_feedback", 1 };
    inline const juce::ParameterID delay_mix        { "delay_mix", 1 };

    // Chorus
    inline const juce::ParameterID chorus_enable    { "chorus_enable", 1 };
    inline const juce::ParameterID chorus_rate      { "chorus_rate", 1 };
    inline const juce::ParameterID chorus_depth     { "chorus_depth", 1 };
    inline const juce::ParameterID chorus_mix       { "chorus_mix", 1 };

    // AutoFilter
    inline const juce::ParameterID filter_enable    { "filter_enable", 1 };
    inline const juce::ParameterID filter_cutoff    { "filter_cutoff", 1 };
    inline const juce::ParameterID filter_res       { "filter_res", 1 };
    inline const juce::ParameterID filter_lfo_rate   { "filter_lfo_rate", 1 };
    inline const juce::ParameterID filter_lfo_depth  { "filter_lfo_depth", 1 };

    // Autopan
    inline const juce::ParameterID pan_enable       { "pan_enable", 1 };
    inline const juce::ParameterID pan_rate         { "pan_rate", 1 };
    inline const juce::ParameterID pan_depth        { "pan_depth", 1 };

    // Halftime
    inline const juce::ParameterID halftime_enable  { "halftime_enable", 1 };
    inline const juce::ParameterID halftime_mix     { "halftime_mix", 1 };
    inline const juce::ParameterID halftime_fade    { "halftime_fade", 1 };

    // Vintage
    inline const juce::ParameterID vintage_enable   { "vintage_enable", 1 };
    inline const juce::ParameterID vintage_wow      { "vintage_wow", 1 };
    inline const juce::ParameterID vintage_flutter  { "vintage_flutter", 1 };
    inline const juce::ParameterID vintage_noise    { "vintage_noise", 1 };

    // Saturation
    inline const juce::ParameterID sat_enable       { "sat_enable", 1 };
    inline const juce::ParameterID sat_drive        { "sat_drive", 1 };
    inline const juce::ParameterID sat_type         { "sat_type", 1 };
    inline const juce::ParameterID sat_mix          { "sat_mix", 1 };
}
