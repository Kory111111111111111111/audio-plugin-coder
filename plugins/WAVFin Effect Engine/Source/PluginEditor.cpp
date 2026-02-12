#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
WAVFinEffectEngineAudioProcessorEditor::WAVFinEffectEngineAudioProcessorEditor (WAVFinEffectEngineAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set editor size to match UI design
    setSize (900, 750);

    // 1. Define parameter getter
    auto& apvts = audioProcessor.apvts;
    auto getParam = [&apvts](const juce::ParameterID& id) -> juce::RangedAudioParameter& {
        auto* p = apvts.getParameter(id.getParamID());
        jassert(p != nullptr);
        return *p;
    };

    // 2. Create attachments (MUST be before webView to sync initial state)
    //    We initialize these first so the Relays contain the correct current parameter values
    //    when they are passed to the WebView Options below.
    globalMixAttachment        = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::global_mix), globalMixRelay, nullptr);
    outputGainAttachment       = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::output_gain), outputGainRelay, nullptr);

    reverbEnableAttachment     = std::make_unique<juce::WebToggleButtonParameterAttachment>(getParam(ParameterIDs::reverb_enable), reverbEnableRelay, nullptr);
    reverbSizeAttachment       = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::reverb_size), reverbSizeRelay, nullptr);
    reverbDecayAttachment      = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::reverb_decay), reverbDecayRelay, nullptr);
    reverbMixAttachment        = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::reverb_mix), reverbMixRelay, nullptr);

    delayEnableAttachment      = std::make_unique<juce::WebToggleButtonParameterAttachment>(getParam(ParameterIDs::delay_enable), delayEnableRelay, nullptr);
    delayTimeAttachment        = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::delay_time), delayTimeRelay, nullptr);
    delayFeedbackAttachment    = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::delay_feedback), delayFeedbackRelay, nullptr);
    delayMixAttachment         = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::delay_mix), delayMixRelay, nullptr);

    chorusEnableAttachment     = std::make_unique<juce::WebToggleButtonParameterAttachment>(getParam(ParameterIDs::chorus_enable), chorusEnableRelay, nullptr);
    chorusRateAttachment       = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::chorus_rate), chorusRateRelay, nullptr);
    chorusDepthAttachment      = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::chorus_depth), chorusDepthRelay, nullptr);
    chorusMixAttachment        = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::chorus_mix), chorusMixRelay, nullptr);

    filterEnableAttachment     = std::make_unique<juce::WebToggleButtonParameterAttachment>(getParam(ParameterIDs::filter_enable), filterEnableRelay, nullptr);
    filterCutoffAttachment     = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::filter_cutoff), filterCutoffRelay, nullptr);
    filterResAttachment        = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::filter_res), filterResRelay, nullptr);
    filterLfoRateAttachment    = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::filter_lfo_rate), filterLfoRateRelay, nullptr);
    filterLfoDepthAttachment   = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::filter_lfo_depth), filterLfoDepthRelay, nullptr);

    panEnableAttachment        = std::make_unique<juce::WebToggleButtonParameterAttachment>(getParam(ParameterIDs::pan_enable), panEnableRelay, nullptr);
    panRateAttachment          = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::pan_rate), panRateRelay, nullptr);
    panDepthAttachment         = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::pan_depth), panDepthRelay, nullptr);

    halftimeEnableAttachment   = std::make_unique<juce::WebToggleButtonParameterAttachment>(getParam(ParameterIDs::halftime_enable), halftimeEnableRelay, nullptr);
    halftimeMixAttachment      = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::halftime_mix), halftimeMixRelay, nullptr);
    halftimeFadeAttachment     = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::halftime_fade), halftimeFadeRelay, nullptr);

    vintageEnableAttachment    = std::make_unique<juce::WebToggleButtonParameterAttachment>(getParam(ParameterIDs::vintage_enable), vintageEnableRelay, nullptr);
    vintageWowAttachment       = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::vintage_wow), vintageWowRelay, nullptr);
    vintageFlutterAttachment   = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::vintage_flutter), vintageFlutterRelay, nullptr);
    vintageNoiseAttachment     = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::vintage_noise), vintageNoiseRelay, nullptr);

    satEnableAttachment        = std::make_unique<juce::WebToggleButtonParameterAttachment>(getParam(ParameterIDs::sat_enable), satEnableRelay, nullptr);
    satDriveAttachment         = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::sat_drive), satDriveRelay, nullptr);
    satTypeAttachment          = std::make_unique<juce::WebComboBoxParameterAttachment>(getParam(ParameterIDs::sat_type), satTypeRelay, nullptr);
    satMixAttachment           = std::make_unique<juce::WebSliderParameterAttachment>(getParam(ParameterIDs::sat_mix), satMixRelay, nullptr);

    // 3. Initialize WebView (Now Relays are populated)
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options()
            .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
            .withWinWebView2Options (juce::WebBrowserComponent::Options::WinWebView2()
                .withUserDataFolder (juce::File::getSpecialLocation (juce::File::tempDirectory)))
            .withResourceProvider ([this] (const auto& url) { return getResource (url); })
            .withNativeIntegrationEnabled()
            .withOptionsFrom(globalMixRelay)
            .withOptionsFrom(outputGainRelay)
            .withOptionsFrom(reverbEnableRelay)
            .withOptionsFrom(reverbSizeRelay)
            .withOptionsFrom(reverbDecayRelay)
            .withOptionsFrom(reverbMixRelay)
            .withOptionsFrom(delayEnableRelay)
            .withOptionsFrom(delayTimeRelay)
            .withOptionsFrom(delayFeedbackRelay)
            .withOptionsFrom(delayMixRelay)
            .withOptionsFrom(chorusEnableRelay)
            .withOptionsFrom(chorusRateRelay)
            .withOptionsFrom(chorusDepthRelay)
            .withOptionsFrom(chorusMixRelay)
            .withOptionsFrom(filterEnableRelay)
            .withOptionsFrom(filterCutoffRelay)
            .withOptionsFrom(filterResRelay)
            .withOptionsFrom(filterLfoRateRelay)
            .withOptionsFrom(filterLfoDepthRelay)
            .withOptionsFrom(panEnableRelay)
            .withOptionsFrom(panRateRelay)
            .withOptionsFrom(panDepthRelay)
            .withOptionsFrom(halftimeEnableRelay)
            .withOptionsFrom(halftimeMixRelay)
            .withOptionsFrom(halftimeFadeRelay)
            .withOptionsFrom(vintageEnableRelay)
            .withOptionsFrom(vintageWowRelay)
            .withOptionsFrom(vintageFlutterRelay)
            .withOptionsFrom(vintageNoiseRelay)
            .withOptionsFrom(satEnableRelay)
            .withOptionsFrom(satDriveRelay)
            .withOptionsFrom(satTypeRelay)
            .withOptionsFrom(satMixRelay)
    );

    addAndMakeVisible(*webView);

    // 4. Load UI
    webView->goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    // 5. Ensure initial layout is correct
    resized();
}

WAVFinEffectEngineAudioProcessorEditor::~WAVFinEffectEngineAudioProcessorEditor()
{
}

//==============================================================================
void WAVFinEffectEngineAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::greyLevel (0.1f));
}

void WAVFinEffectEngineAudioProcessorEditor::resized()
{
    if (webView)
        webView->setBounds(getLocalBounds());
}

//==============================================================================
// Resource Provider Implementation

const char* WAVFinEffectEngineAudioProcessorEditor::getMimeForExtension (const juce::String& extension)
{
    static const std::unordered_map<juce::String, const char*> mimeMap =
    {
        { "html", "text/html" },
        { "css",  "text/css" },
        { "js",   "text/javascript" },
        { "mjs",  "text/javascript" },
        { "json", "application/json" },
        { "png",  "image/png" },
        { "jpg",  "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "svg",  "image/svg+xml" }
    };

    auto it = mimeMap.find(extension.toLowerCase());
    if (it != mimeMap.end())
        return it->second;

    return "text/plain";
}

juce::String WAVFinEffectEngineAudioProcessorEditor::getExtension (juce::String filename)
{
    return filename.fromLastOccurrenceOf(".", false, false);
}

std::optional<juce::WebBrowserComponent::Resource> WAVFinEffectEngineAudioProcessorEditor::getResource (const juce::String& url)
{
    juce::String requestPath = url;

    // 1. Strip protocol (e.g. "https://")
    if (requestPath.contains ("://"))
        requestPath = requestPath.fromFirstOccurrenceOf ("://", false, false);

    // 2. Strip host (e.g. "juce.backend")
    if (requestPath.contains ("/"))
        requestPath = requestPath.fromFirstOccurrenceOf ("/", false, false);
    else
        requestPath = "";

    // 3. Strip query parameters and fragments
    if (requestPath.contains ("?")) requestPath = requestPath.upToFirstOccurrenceOf ("?", false, false);
    if (requestPath.contains ("#")) requestPath = requestPath.upToFirstOccurrenceOf ("#", false, false);

    // 4. Map root to index.html
    if (requestPath.isEmpty() || requestPath == "/")
        requestPath = "index.html";

    // 5. Clean up leading slashes
    while (requestPath.startsWith ("/"))
        requestPath = requestPath.substring (1);

    // 1. Try to find the resource in our local BinaryData namespace
    for (int i = 0; i < WAVFinWebData::namedResourceListSize; ++i)
    {
        const char* resourceName = WAVFinWebData::namedResourceList[i];
        const char* originalFilename = WAVFinWebData::getNamedResourceOriginalFilename (resourceName);

        if (originalFilename == nullptr) continue;

        juce::String resourceFile (originalFilename);

        if (requestPath == resourceFile 
            || requestPath.endsWith ("/" + resourceFile) 
            || requestPath.endsWith ("\\" + resourceFile))
        {
            int dataSize = 0;
            const char* data = WAVFinWebData::getNamedResource (resourceName, dataSize);

            if (data != nullptr && dataSize > 0)
            {
                std::vector<std::byte> byteData ((size_t) dataSize);
                std::memcpy (byteData.data(), data, (size_t) dataSize);
                auto mime = getMimeForExtension (getExtension (requestPath).toLowerCase());
                return juce::WebBrowserComponent::Resource { std::move (byteData), juce::String { mime } };
            }
        }
    }

    // 2. HARD FALLBACK: If nothing matches and it looks like a root request, return index.html
    if (requestPath == "index.html")
    {
        int dataSize = 0;
        const char* data = WAVFinWebData::getNamedResource ("index_html", dataSize);
        if (data != nullptr && dataSize > 0)
        {
             std::vector<std::byte> byteData ((size_t) dataSize);
             std::memcpy (byteData.data(), data, (size_t) dataSize);
             return juce::WebBrowserComponent::Resource { std::move (byteData), "text/html" };
        }
    }

    return std::nullopt;
}
