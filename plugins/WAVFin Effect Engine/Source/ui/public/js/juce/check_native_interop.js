/**
 * check_native_interop.js
 * 
 * Verifies that the JUCE native integration is working correctly.
 * This is useful for debugging why controls might not be updating.
 */

if (typeof window.__JUCE__ === 'undefined') {
    console.error("CRITICAL: window.__JUCE__ is undefined!");
    console.error("This means the WebView is not connected to the C++ backend.");
    console.error("Troubleshooting:");
    console.error("1. Ensure .withNativeIntegrationEnabled() is called in C++");
    console.error("2. Ensure .withResourceProvider() is correctly set up");
    console.error("3. Check if 'modules/juce_gui_extra/native/javascript/index.js' is loaded");
} else {
    console.log("✅ JUCE Native Integration Active");
    console.log("Backend version:", window.__JUCE__.backendVersion || "Unknown");
}
