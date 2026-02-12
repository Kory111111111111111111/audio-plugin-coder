import * as Juce from "./juce/juce_core.js";

// Main entry point
document.addEventListener("DOMContentLoaded", () => {
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

    // Safety check just in case
    if (typeof Juce === 'undefined' || Juce === null) {
        console.warn("Juce module not loaded correctly.");
    } else {
        console.log("Juce module detected");
    }
});

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
 * Initialize toggle switches
 */
function initializeToggles() {
    const toggles = document.querySelectorAll('.toggle');

    toggles.forEach(toggle => {
        const paramId = toggle.dataset.param;
        const juceState = Juce.getToggleState(paramId); // Corresponds to WebToggleRelay

        const updateVisuals = (isActive) => {
            if (isActive) {
                toggle.classList.add('active');
            } else {
                toggle.classList.remove('active');
            }

            // Light up parent module
            const module = toggle.closest('.module-card');
            if (module) {
                if (isActive) module.classList.add('active');
                else module.classList.remove('active');
            }
        };

        // JUCE Listener
        if (juceState) {
            juceState.valueChangedEvent.addListener((newValue) => {
                updateVisuals(newValue);
            });
            // Initial sync
            updateVisuals(juceState.getValue());
        }

        // Interaction
        toggle.addEventListener('mousedown', function (e) {
            e.preventDefault(); // Prevent double triggers on some devices

            // Current visual state
            const isCurrentlyActive = toggle.classList.contains('active');
            const newState = !isCurrentlyActive;

            console.log(`Toggle ${paramId} clicked: ${newState}`);

            // Optimistic update
            updateVisuals(newState);

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

    const updateVisuals = (index) => {
        if (index >= 0 && index < satTypes.length) {
            satDisplay.textContent = satTypes[index];
        }
    };

    if (juceState) {
        juceState.valueChangedEvent.addListener((val) => {
            // value is normalized 0.0-1.0 in ComboBoxState.value property, 
            // but for ComboBoxState we usually care about the index.
            // Wait, ComboBoxState.handleEvent updates this.value
            // And this.value is used by getChoiceIndex().
            // But valueChangedEvent passes 'this.value' (normalized).
            // So we should call getChoiceIndex() to be sure, or map it.
            updateVisuals(juceState.getChoiceIndex());
        });
        updateVisuals(juceState.getChoiceIndex());
    }

    satDisplay.onclick = function () {
        // Get current index
        let currentIndex = juceState ? juceState.getChoiceIndex() : 0;
        // If not connected, we need to store state locally, but for now assuming logic works
        // Fallback if juceState is null (UI testing)
        if (!juceState) {
            // Find current text in array
            currentIndex = satTypes.indexOf(satDisplay.textContent);
            if (currentIndex === -1) currentIndex = 0;
        }

        const nextIndex = (currentIndex + 1) % satTypes.length;

        // Optimistic update
        updateVisuals(nextIndex);

        // Notify JUCE
        if (juceState) {
            juceState.setChoiceIndex(nextIndex);
        }
    };
}
