import * as Juce from "./juce/juce_core.js";

// Main entry point
document.addEventListener("DOMContentLoaded", async () => {
    console.log("DOM Content Loaded - Initializing UI...");

    try {
        initializeKnobs();
        console.log("Knobs initialized");
    } catch (e) {
        console.error("Failed to initialize knobs:", e);
    }

    try {
        initializeToggles();
        console.log("Toggles initialized");
    } catch (e) {
        console.error("Failed to initialize toggles:", e);
    }

    try {
        initializeSelectors();
        console.log("Selectors initialized");
    } catch (e) {
        console.error("Failed to initialize selectors:", e);
    }

    // Fetch parameter values from C++ (bypasses event visibility issues on window reopen)
    try {
        if (typeof Juce !== 'undefined' && Juce !== null &&
            window.__JUCE__?.initialisationData?.__juce__functions?.includes?.("getAllParameterValues")) {
            const getParams = Juce.getNativeFunction("getAllParameterValues");
            const values = await getParams();
            if (values && typeof values === 'object') {
                applyParameterValues(values);
            }
        }
    } catch (e) {
        console.warn("Could not fetch initial parameter values:", e);
    }

    if (typeof Juce === 'undefined' || Juce === null) {
        console.warn("Juce module not loaded correctly.");
    } else {
        console.log("Juce module detected");
    }
});

/** Apply parameter values from C++ backend (normalised 0-1). Bypasses event system. */
function applyParameterValues(values) {
    const sliderParams = [
        "global_mix", "output_gain", "reverb_size", "reverb_decay", "reverb_mix",
        "delay_time", "delay_feedback", "delay_mix", "chorus_rate", "chorus_depth", "chorus_mix",
        "filter_cutoff", "filter_res", "filter_lfo_rate", "filter_lfo_depth",
        "pan_rate", "pan_depth", "halftime_mix", "halftime_fade",
        "vintage_wow", "vintage_flutter", "vintage_noise", "sat_drive", "sat_mix"
    ];
    const toggleParams = [
        "reverb_enable", "delay_enable", "chorus_enable", "filter_enable", "pan_enable",
        "halftime_enable", "vintage_enable", "sat_enable"
    ];
    for (const id of sliderParams) {
        const v = values[id];
        if (typeof v === 'number') {
            const s = Juce.getSliderState(id);
            if (s) s.setNormalisedValue(v);
        }
    }
    for (const id of toggleParams) {
        const v = values[id];
        if (typeof v === 'number') {
            const t = Juce.getToggleState(id);
            if (t) t.setValue(v > 0.5);
        }
    }
    const satType = values["sat_type"];
    if (typeof satType === 'number') {
        const c = Juce.getComboBoxState("sat_type");
        if (c) c.setChoiceIndex(Math.round(satType * 3));
    }
}

/**
 * Utility class for "Interaction Locking".
 * Enforces a hard visual lock for a set duration after user interaction.
 * Ignores ALL contradictory backend updates during this period.
 * This prevents "flicker/revert" issues caused by async race conditions.
 */
class InteractionLock {
    constructor(updateVisualsFn, lockDurationMs = 500) {
        this.updateVisuals = updateVisualsFn;
        this.lockDurationMs = lockDurationMs;
        this.lockedUntil = 0;
        this.targetValue = null;
    }

    /** Call this when the USER interacts */
    onUserAction(newValue) {
        // 1. Apply User Intent Immediately
        this.updateVisuals(newValue);

        // 2. Lock the UI to this value
        this.targetValue = newValue;
        this.lockedUntil = Date.now() + this.lockDurationMs;

        // console.log(`[Lock] UI Locked to ${newValue} for ${this.lockDurationMs}ms`);
    }

    /** Call this when the BACKEND sends an event */
    onBackendUpdate(incomingValue) {
        // Check if locked
        if (Date.now() < this.lockedUntil) {
            // We are in the protected window.
            // If the incoming value contradicts our target, IGNORE it.
            if (incomingValue !== this.targetValue) {
                // console.warn(`[Lock] Ignored conflicting update: ${incomingValue} (Target: ${this.targetValue})`);
                return;
            }
            // If it matches, we allow it (it's consistent), but we keep the lock 
            // to absorb any subsequent contradictory glitches/echoes.
        }

        // Unlocked or Consistent -> Apply Update
        this.updateVisuals(incomingValue);
    }
}

/**
 * Initialize all knob controls
 */
function initializeKnobs() {
    const knobs = document.querySelectorAll('.knob-container');

    knobs.forEach(knob => {
        // Create SVG structure with dot for indicator
        knob.innerHTML = `
            <svg class="knob-svg" viewBox="0 0 36 36">
                <path class="knob-track" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" stroke-dasharray="75, 25" />
                <path class="knob-value-arc" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" stroke-dasharray="0, 100" />
                <circle class="knob-dot" cx="18" cy="2.0845" r="1.5" transform-origin="18 18" />
            </svg>
            <div class="knob-text"></div>
        `;

        const paramId = knob.dataset.param;
        const juceState = Juce.getSliderState(paramId);

        const state = {
            isDragging: false,
            startY: 0,
            startValue: parseFloat(knob.dataset.value),
            min: parseFloat(knob.dataset.min),
            max: parseFloat(knob.dataset.max),
            value: parseFloat(knob.dataset.value),
            suffix: knob.dataset.suffix || '',
            isLog: knob.dataset.log === "true"
        };

        // --- Visual Update Logic ---
        const updateVisuals = () => {
            const currentVal = typeof state.value === 'number' ? state.value : state.min;

            // 1. Arc Length & Percent
            const range = state.max - state.min;
            let percent = range > 0 ? (currentVal - state.min) / range : 0;
            percent = Math.max(0, Math.min(1, percent));

            const arcLength = percent * 75;
            const arc = knob.querySelector('.knob-value-arc');
            if (arc) arc.style.strokeDasharray = `${arcLength}, 100`;

            // 2. Rotate Indicator Dot
            const dot = knob.querySelector('.knob-dot');
            if (dot) {
                // Adjust rotation: starts at top (0 deg). 
                // Arc starts at -90 deg (left-ish) and is 270 deg total (75% of circle).
                // Wait, our SVG path starts at the top (18, 2.0845).
                // So 0 percent is at the top? No, the arc path is circular.
                // percent * 270 degrees.
                const rotation = percent * 270;
                // Move it slightly so 0% starts at bottom-left
                dot.style.transform = `rotate(${rotation}deg)`;
            }

            // 3. Text Label Formatting
            const text = knob.querySelector('.knob-text');
            if (text) {
                if (isNaN(currentVal)) {
                    text.textContent = "--";
                } else if (currentVal >= 1000) {
                    text.textContent = (currentVal / 1000).toFixed(1) + 'k';
                } else if (paramId === "output_gain" || paramId === "sat_drive") {
                    // Show one decimal for gain/drive
                    text.textContent = currentVal.toFixed(1);
                } else if (Math.abs(currentVal) < 10 && currentVal % 1 !== 0) {
                    text.textContent = currentVal.toFixed(1);
                } else {
                    text.textContent = Math.round(currentVal);
                }
            }
        };

        // --- JUCE Listener ---
        if (juceState) {
            juceState.valueChangedEvent.addListener(() => {
                // If dragging, we prioritize local interaction
                if (state.isDragging) return;

                state.value = juceState.getScaledValue();
                updateVisuals();
            });
            state.value = juceState.getScaledValue();
            updateVisuals();
        } else {
            updateVisuals();
        }

        // --- Interaction Logic ---
        knob.addEventListener('mousedown', (e) => {
            state.isDragging = true;
            state.startY = e.clientY;
            state.startValue = state.value;
            if (juceState && juceState.sliderDragStarted) juceState.sliderDragStarted();
            document.body.style.cursor = 'ns-resize';
            e.preventDefault();
        });

        window.addEventListener('mousemove', (e) => {
            if (!state.isDragging) return;

            const deltaY = state.startY - e.clientY;
            const range = state.max - state.min;

            // Logarithmic mapping for frequency (if enabled)
            let newValue;
            if (state.isLog) {
                // Simplified log mapping
                const logMin = Math.log(state.min);
                const logMax = Math.log(state.max);
                const currentLog = Math.log(state.value);
                const logDelta = (deltaY / 200) * (logMax - logMin);
                newValue = Math.exp(currentLog + logDelta);
            } else {
                const sensitivity = range / 200;
                newValue = state.startValue + (deltaY * sensitivity);
            }

            newValue = Math.max(state.min, Math.min(state.max, newValue));

            state.value = newValue;
            updateVisuals();

            if (juceState) {
                window.__JUCE__.backend.emitEvent(juceState.identifier, {
                    eventType: "valueChanged",
                    value: newValue
                });
            }
        });

        window.addEventListener('mouseup', () => {
            if (!state.isDragging) return;
            state.isDragging = false;
            if (juceState && juceState.sliderDragEnded) juceState.sliderDragEnded();
            document.body.style.cursor = 'default';
        });
    });
}

/**
 * Initialize toggle switches with robust state handling
 */
function initializeToggles() {
    const toggles = document.querySelectorAll('.toggle');

    toggles.forEach(toggle => {
        const paramId = toggle.dataset.param;
        const juceState = Juce.getToggleState(paramId);

        // Visual update logic
        const updateDom = (isActive) => {
            const active = !!isActive;
            if (active) toggle.classList.add('active');
            else toggle.classList.remove('active');

            const module = toggle.closest('.module-card');
            if (module) {
                if (active) module.classList.add('active');
                else module.classList.remove('active');
            }
        };

        // Create Sync Manager
        const lock = new InteractionLock(updateDom, 500);

        // JUCE Listener
        if (juceState) {
            juceState.valueChangedEvent.addListener((newValue) => {
                lock.onBackendUpdate(!!newValue);
            });
            // Initial sync (bypass optimistic logic)
            updateDom(juceState.getValue());
        }

        // Interaction
        toggle.addEventListener('click', function (e) {
            e.preventDefault();
            e.stopPropagation();

            const isCurrentlyActive = toggle.classList.contains('active');
            const newState = !isCurrentlyActive;

            console.log(`[Interaction] ${paramId}: ${newState}`);

            // Notify Sync Manager
            lock.onUserAction(newState);

            // Notify JUCE
            if (juceState) {
                juceState.setValue(newState);
            }
        });
    });
}

/**
 * Initialize selector (Sat Type)
 */
function initializeSelectors() {
    const satDisplay = document.getElementById('sat_type_display');
    if (!satDisplay) return;

    // Use WebComboBoxRelay for choice parameters
    const paramId = "sat_type";
    const juceState = Juce.getComboBoxState(paramId);

    const satTypes = ['TUBE', 'TAPE', 'DIODE', 'DIGI'];

    // Visual Update
    const updateDom = (index) => {
        if (index >= 0 && index < satTypes.length) {
            satDisplay.textContent = satTypes[index];
        }
    };

    // Sync Manager (Values are integers 0-3)
    const lock = new InteractionLock(updateDom, 500);

    if (juceState) {
        juceState.valueChangedEvent.addListener(() => {
            // ComboBoxState updates 'value' (0.0-1.0) internally before this event.
            // But getChoiceIndex() derives from it.
            lock.onBackendUpdate(juceState.getChoiceIndex());
        });
        updateDom(juceState.getChoiceIndex());
    }

    satDisplay.onclick = function (e) {
        e.preventDefault();
        e.stopPropagation();

        let currentIndex = juceState ? juceState.getChoiceIndex() : 0;
        // Fallback for independent testing
        if (!juceState) {
            currentIndex = satTypes.indexOf(satDisplay.textContent);
            if (currentIndex === -1) currentIndex = 0;
        }

        const nextIndex = (currentIndex + 1) % satTypes.length;

        // Notify Sync Manager
        lock.onUserAction(nextIndex);

        // Notify JUCE
        if (juceState) {
            juceState.setChoiceIndex(nextIndex);
        }
    };
}
