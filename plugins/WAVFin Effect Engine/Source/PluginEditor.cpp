#include "PluginEditor.h"
#include "BinaryData.h"
#include "PluginProcessor.h"

//==============================================================================
void WAVFinWebView::pageFinishedLoading(const juce::String& url) {
  juce::WebBrowserComponent::pageFinishedLoading(url);
  if (onPageLoaded)
    onPageLoaded(url);
}

//==============================================================================
WAVFinEffectEngineAudioProcessorEditor::WAVFinEffectEngineAudioProcessorEditor(
    WAVFinEffectEngineAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  // Set editor size to match UI design
  setSize(900, 750);

  // 1. Define parameter getter
  auto &apvts = audioProcessor.apvts;
  auto getParam =
      [&apvts](const juce::ParameterID &id) -> juce::RangedAudioParameter & {
    auto *p = apvts.getParameter(id.getParamID());
    jassert(p != nullptr);
    return *p;
  };

  // 2. Create attachments (MUST be before webView to sync initial state)
  //    We initialize these first so the Relays contain the correct current
  //    parameter values when they are passed to the WebView Options below.
  globalMixAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::global_mix), globalMixRelay, nullptr);
  outputGainAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::output_gain), outputGainRelay, nullptr);

  reverbEnableAttachment =
      std::make_unique<juce::WebToggleButtonParameterAttachment>(
          getParam(ParameterIDs::reverb_enable), reverbEnableRelay, nullptr);
  reverbSizeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::reverb_size), reverbSizeRelay, nullptr);
  reverbDecayAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::reverb_decay), reverbDecayRelay, nullptr);
  reverbMixAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::reverb_mix), reverbMixRelay, nullptr);

  delayEnableAttachment =
      std::make_unique<juce::WebToggleButtonParameterAttachment>(
          getParam(ParameterIDs::delay_enable), delayEnableRelay, nullptr);
  delayTimeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::delay_time), delayTimeRelay, nullptr);
  delayFeedbackAttachment =
      std::make_unique<juce::WebSliderParameterAttachment>(
          getParam(ParameterIDs::delay_feedback), delayFeedbackRelay, nullptr);
  delayMixAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::delay_mix), delayMixRelay, nullptr);

  chorusEnableAttachment =
      std::make_unique<juce::WebToggleButtonParameterAttachment>(
          getParam(ParameterIDs::chorus_enable), chorusEnableRelay, nullptr);
  chorusRateAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::chorus_rate), chorusRateRelay, nullptr);
  chorusDepthAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::chorus_depth), chorusDepthRelay, nullptr);
  chorusMixAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::chorus_mix), chorusMixRelay, nullptr);

  filterEnableAttachment =
      std::make_unique<juce::WebToggleButtonParameterAttachment>(
          getParam(ParameterIDs::filter_enable), filterEnableRelay, nullptr);
  filterCutoffAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::filter_cutoff), filterCutoffRelay, nullptr);
  filterResAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::filter_res), filterResRelay, nullptr);
  filterLfoRateAttachment =
      std::make_unique<juce::WebSliderParameterAttachment>(
          getParam(ParameterIDs::filter_lfo_rate), filterLfoRateRelay, nullptr);
  filterLfoDepthAttachment =
      std::make_unique<juce::WebSliderParameterAttachment>(
          getParam(ParameterIDs::filter_lfo_depth), filterLfoDepthRelay,
          nullptr);

  panEnableAttachment =
      std::make_unique<juce::WebToggleButtonParameterAttachment>(
          getParam(ParameterIDs::pan_enable), panEnableRelay, nullptr);
  panRateAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::pan_rate), panRateRelay, nullptr);
  panDepthAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::pan_depth), panDepthRelay, nullptr);

  halftimeEnableAttachment =
      std::make_unique<juce::WebToggleButtonParameterAttachment>(
          getParam(ParameterIDs::halftime_enable), halftimeEnableRelay,
          nullptr);
  halftimeMixAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::halftime_mix), halftimeMixRelay, nullptr);
  halftimeFadeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::halftime_fade), halftimeFadeRelay, nullptr);

  vintageEnableAttachment =
      std::make_unique<juce::WebToggleButtonParameterAttachment>(
          getParam(ParameterIDs::vintage_enable), vintageEnableRelay, nullptr);
  vintageWowAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::vintage_wow), vintageWowRelay, nullptr);
  vintageFlutterAttachment =
      std::make_unique<juce::WebSliderParameterAttachment>(
          getParam(ParameterIDs::vintage_flutter), vintageFlutterRelay,
          nullptr);
  vintageNoiseAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::vintage_noise), vintageNoiseRelay, nullptr);

  satEnableAttachment =
      std::make_unique<juce::WebToggleButtonParameterAttachment>(
          getParam(ParameterIDs::sat_enable), satEnableRelay, nullptr);
  satDriveAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::sat_drive), satDriveRelay, nullptr);
  satTypeAttachment = std::make_unique<juce::WebComboBoxParameterAttachment>(
      getParam(ParameterIDs::sat_type), satTypeRelay, nullptr);
  satMixAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
      getParam(ParameterIDs::sat_mix), satMixRelay, nullptr);

  // FIX: Manually sync Relays to Parameter values to ensure initial state is
  // correct in WebView This addresses the "UI defaults to zero" bug on window
  // reopen.
  auto syncSlider = [&](juce::WebSliderRelay &relay,
                        const juce::ParameterID &pid) {
    if (auto *param = audioProcessor.apvts.getParameter(pid.getParamID()))
      relay.setValue(param->convertFrom0to1(param->getValue()));
  };

  auto syncToggle = [&](juce::WebToggleButtonRelay &relay,
                        const juce::ParameterID &pid) {
    if (auto *param = audioProcessor.apvts.getParameter(pid.getParamID()))
      relay.setToggleState(param->getValue() > 0.5f);
  };

  syncSlider(globalMixRelay, ParameterIDs::global_mix);
  syncSlider(outputGainRelay, ParameterIDs::output_gain);

  syncToggle(reverbEnableRelay, ParameterIDs::reverb_enable);
  syncSlider(reverbSizeRelay, ParameterIDs::reverb_size);
  syncSlider(reverbDecayRelay, ParameterIDs::reverb_decay);
  syncSlider(reverbMixRelay, ParameterIDs::reverb_mix);

  syncToggle(delayEnableRelay, ParameterIDs::delay_enable);
  syncSlider(delayTimeRelay, ParameterIDs::delay_time);
  syncSlider(delayFeedbackRelay, ParameterIDs::delay_feedback);
  syncSlider(delayMixRelay, ParameterIDs::delay_mix);

  syncToggle(chorusEnableRelay, ParameterIDs::chorus_enable);
  syncSlider(chorusRateRelay, ParameterIDs::chorus_rate);
  syncSlider(chorusDepthRelay, ParameterIDs::chorus_depth);
  syncSlider(chorusMixRelay, ParameterIDs::chorus_mix);

  syncToggle(filterEnableRelay, ParameterIDs::filter_enable);
  syncSlider(filterCutoffRelay, ParameterIDs::filter_cutoff);
  syncSlider(filterResRelay, ParameterIDs::filter_res);
  syncSlider(filterLfoRateRelay, ParameterIDs::filter_lfo_rate);
  syncSlider(filterLfoDepthRelay, ParameterIDs::filter_lfo_depth);

  syncToggle(panEnableRelay, ParameterIDs::pan_enable);
  syncSlider(panRateRelay, ParameterIDs::pan_rate);
  syncSlider(panDepthRelay, ParameterIDs::pan_depth);

  syncToggle(halftimeEnableRelay, ParameterIDs::halftime_enable);
  syncSlider(halftimeMixRelay, ParameterIDs::halftime_mix);
  syncSlider(halftimeFadeRelay, ParameterIDs::halftime_fade);

  syncToggle(vintageEnableRelay, ParameterIDs::vintage_enable);
  syncSlider(vintageWowRelay, ParameterIDs::vintage_wow);
  syncSlider(vintageFlutterRelay, ParameterIDs::vintage_flutter);
  syncSlider(vintageNoiseRelay, ParameterIDs::vintage_noise);

  syncToggle(satEnableRelay, ParameterIDs::sat_enable);
  syncSlider(satDriveRelay, ParameterIDs::sat_drive);
  syncSlider(satMixRelay, ParameterIDs::sat_mix);

  if (auto *param = audioProcessor.apvts.getParameter(
          ParameterIDs::sat_type.getParamID()))
    satTypeRelay.setValue(param->getValue());

  // 3. Initialize WebView (Now Relays are populated)
  // Native function bypasses emitEventIfBrowserIsVisible - frontend fetches values when ready
  auto getParamValues = [this](const juce::Array<juce::var> &,
                               juce::WebBrowserComponent::NativeFunctionCompletion completion) {
    juce::DynamicObject::Ptr obj(new juce::DynamicObject);
    auto &apvts = audioProcessor.apvts;
    auto addParam = [&](const juce::ParameterID &id) {
      if (auto *p = apvts.getParameter(id.getParamID()))
        obj->setProperty(id.getParamID(), p->getValue());
    };
    addParam(ParameterIDs::global_mix);
    addParam(ParameterIDs::output_gain);
    addParam(ParameterIDs::reverb_enable);
    addParam(ParameterIDs::reverb_size);
    addParam(ParameterIDs::reverb_decay);
    addParam(ParameterIDs::reverb_mix);
    addParam(ParameterIDs::delay_enable);
    addParam(ParameterIDs::delay_time);
    addParam(ParameterIDs::delay_feedback);
    addParam(ParameterIDs::delay_mix);
    addParam(ParameterIDs::chorus_enable);
    addParam(ParameterIDs::chorus_rate);
    addParam(ParameterIDs::chorus_depth);
    addParam(ParameterIDs::chorus_mix);
    addParam(ParameterIDs::filter_enable);
    addParam(ParameterIDs::filter_cutoff);
    addParam(ParameterIDs::filter_res);
    addParam(ParameterIDs::filter_lfo_rate);
    addParam(ParameterIDs::filter_lfo_depth);
    addParam(ParameterIDs::pan_enable);
    addParam(ParameterIDs::pan_rate);
    addParam(ParameterIDs::pan_depth);
    addParam(ParameterIDs::halftime_enable);
    addParam(ParameterIDs::halftime_mix);
    addParam(ParameterIDs::halftime_fade);
    addParam(ParameterIDs::vintage_enable);
    addParam(ParameterIDs::vintage_wow);
    addParam(ParameterIDs::vintage_flutter);
    addParam(ParameterIDs::vintage_noise);
    addParam(ParameterIDs::sat_enable);
    addParam(ParameterIDs::sat_drive);
    addParam(ParameterIDs::sat_type);
    addParam(ParameterIDs::sat_mix);
    completion(juce::var(obj.get()));
  };

  auto opts = juce::WebBrowserComponent::Options()
          .withNativeFunction("getAllParameterValues", getParamValues)
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(
              juce::WebBrowserComponent::Options::WinWebView2()
                  .withUserDataFolder(juce::File::getSpecialLocation(
                      juce::File::tempDirectory)))
          .withResourceProvider(
              [this](const auto &url) { return getResource(url); })
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
          .withOptionsFrom(satMixRelay);
  webView = std::make_unique<WAVFinWebView>(opts);
  webView->onPageLoaded = [this](const juce::String& /*url*/) { syncParametersToWebView(); };

  addAndMakeVisible(*webView);

  // 4. Load UI
  webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

  // 5. Ensure initial layout is correct
  resized();
}

WAVFinEffectEngineAudioProcessorEditor::
    ~WAVFinEffectEngineAudioProcessorEditor() {
  stopTimer();
}

//==============================================================================
void WAVFinEffectEngineAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour::greyLevel(0.1f));
}

void WAVFinEffectEngineAudioProcessorEditor::resized() {
  if (webView)
    webView->setBounds(getLocalBounds());
}

void WAVFinEffectEngineAudioProcessorEditor::visibilityChanged() {
  juce::AudioProcessorEditor::visibilityChanged();
  // When editor becomes visible, sync parameters to WebView. JUCE drops events if
  // the WebView isn't visible yet. Start a retry timer to catch page load timing.
  if (!isVisible() || !globalMixAttachment)
    return;
  stopTimer();
  syncRetryCount = 0;
  syncParametersToWebView();
  startTimer(100);
}

void WAVFinEffectEngineAudioProcessorEditor::timerCallback() {
  syncParametersToWebView();
  if (++syncRetryCount >= 15)  // Retry for ~1.5s to catch async page load
    stopTimer();
}

void WAVFinEffectEngineAudioProcessorEditor::syncParametersToWebView() {
  if (!globalMixAttachment)
    return;
  globalMixAttachment->sendInitialUpdate();
  outputGainAttachment->sendInitialUpdate();
  reverbEnableAttachment->sendInitialUpdate();
  reverbSizeAttachment->sendInitialUpdate();
  reverbDecayAttachment->sendInitialUpdate();
  reverbMixAttachment->sendInitialUpdate();
  delayEnableAttachment->sendInitialUpdate();
  delayTimeAttachment->sendInitialUpdate();
  delayFeedbackAttachment->sendInitialUpdate();
  delayMixAttachment->sendInitialUpdate();
  chorusEnableAttachment->sendInitialUpdate();
  chorusRateAttachment->sendInitialUpdate();
  chorusDepthAttachment->sendInitialUpdate();
  chorusMixAttachment->sendInitialUpdate();
  filterEnableAttachment->sendInitialUpdate();
  filterCutoffAttachment->sendInitialUpdate();
  filterResAttachment->sendInitialUpdate();
  filterLfoRateAttachment->sendInitialUpdate();
  filterLfoDepthAttachment->sendInitialUpdate();
  panEnableAttachment->sendInitialUpdate();
  panRateAttachment->sendInitialUpdate();
  panDepthAttachment->sendInitialUpdate();
  halftimeEnableAttachment->sendInitialUpdate();
  halftimeMixAttachment->sendInitialUpdate();
  halftimeFadeAttachment->sendInitialUpdate();
  vintageEnableAttachment->sendInitialUpdate();
  vintageWowAttachment->sendInitialUpdate();
  vintageFlutterAttachment->sendInitialUpdate();
  vintageNoiseAttachment->sendInitialUpdate();
  satEnableAttachment->sendInitialUpdate();
  satDriveAttachment->sendInitialUpdate();
  satTypeAttachment->sendInitialUpdate();
  satMixAttachment->sendInitialUpdate();
}

//==============================================================================
// Resource Provider Implementation

const char *WAVFinEffectEngineAudioProcessorEditor::getMimeForExtension(
    const juce::String &extension) {
  static const std::unordered_map<juce::String, const char *> mimeMap = {
      {"html", "text/html"},        {"css", "text/css"},
      {"js", "text/javascript"},    {"mjs", "text/javascript"},
      {"json", "application/json"}, {"png", "image/png"},
      {"jpg", "image/jpeg"},        {"jpeg", "image/jpeg"},
      {"svg", "image/svg+xml"}};

  auto it = mimeMap.find(extension.toLowerCase());
  if (it != mimeMap.end())
    return it->second;

  return "text/plain";
}

juce::String
WAVFinEffectEngineAudioProcessorEditor::getExtension(juce::String filename) {
  return filename.fromLastOccurrenceOf(".", false, false);
}

std::optional<juce::WebBrowserComponent::Resource>
WAVFinEffectEngineAudioProcessorEditor::getResource(const juce::String &url) {
  juce::String requestPath = url;

  // 1. Strip protocol (e.g. "https://")
  if (requestPath.contains("://"))
    requestPath = requestPath.fromFirstOccurrenceOf("://", false, false);

  // 2. Strip host (e.g. "juce.backend")
  if (requestPath.contains("/"))
    requestPath = requestPath.fromFirstOccurrenceOf("/", false, false);
  else
    requestPath = "";

  // 3. Strip query parameters and fragments
  if (requestPath.contains("?"))
    requestPath = requestPath.upToFirstOccurrenceOf("?", false, false);
  if (requestPath.contains("#"))
    requestPath = requestPath.upToFirstOccurrenceOf("#", false, false);

  // 4. Map root to index.html
  if (requestPath.isEmpty() || requestPath == "/")
    requestPath = "index.html";

  // 5. Clean up leading slashes
  while (requestPath.startsWith("/"))
    requestPath = requestPath.substring(1);

  // 1. Try to find the resource in our local BinaryData namespace
  for (int i = 0; i < WAVFinWebData::namedResourceListSize; ++i) {
    const char *resourceName = WAVFinWebData::namedResourceList[i];
    const char *originalFilename =
        WAVFinWebData::getNamedResourceOriginalFilename(resourceName);

    if (originalFilename == nullptr)
      continue;

    juce::String resourceFile(originalFilename);

    if (requestPath == resourceFile ||
        requestPath.endsWith("/" + resourceFile) ||
        requestPath.endsWith("\\" + resourceFile)) {
      int dataSize = 0;
      const char *data =
          WAVFinWebData::getNamedResource(resourceName, dataSize);

      if (data != nullptr && dataSize > 0) {
        std::vector<std::byte> byteData((size_t)dataSize);
        std::memcpy(byteData.data(), data, (size_t)dataSize);
        auto mime =
            getMimeForExtension(getExtension(requestPath).toLowerCase());
        return juce::WebBrowserComponent::Resource{std::move(byteData),
                                                   juce::String{mime}};
      }
    }
  }

  // 2. HARD FALLBACK: If nothing matches and it looks like a root request,
  // return index.html
  if (requestPath == "index.html") {
    int dataSize = 0;
    const char *data = WAVFinWebData::getNamedResource("index_html", dataSize);
    if (data != nullptr && dataSize > 0) {
      std::vector<std::byte> byteData((size_t)dataSize);
      std::memcpy(byteData.data(), data, (size_t)dataSize);
      return juce::WebBrowserComponent::Resource{std::move(byteData),
                                                 "text/html"};
    }
  }

  return std::nullopt;
}
