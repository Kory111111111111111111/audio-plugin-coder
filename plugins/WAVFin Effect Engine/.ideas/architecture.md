# DSP Architecture Specification - WAVFin Effect Engine

## Core Components

### 1. Global Processing
- **Input/Output Logic:** Stereo input/output handling.
- **Dry/Wet Mixer:** Global mix control using `juce::dsp::DryWetMixer`.
- **Output Gain:** Final gain stage.

### 2. Effect Modules (Serial Chain)

#### A. Halftime (Time Stretching)
- **Logic:** Circular buffer recording with read head moving at 0.5x speed.
- ** Smoothing:** Crossfading windows (20-100ms) to avoid clicks at loop points.
- **Components:** `AudioBuffer`, `WriteHead`, `ReadHead`, `WindowFunction`.

#### B. Saturation (Dynamics/Color)
- **Logic:** Waveshaping and harmonic injection.
- **Algorithms:**
    - **Tube:** Soft clipping with asymmetric bias.
    - **Tape:** Hysteresis simulation + slight compression.
    - **Diode:** Harder clipping.
    - **Digital:** Bitcrushing/Sample Rate Reduction.
- **Oversampling:** 2x or 4x oversampling to reduce aliasing (essential for high drive).

#### C. AutoFilter (Spectral)
- **Logic:** State Variable Filter (SVF) with LFO modulation.
- **Modulation:** LFO (Sine/Triangle) -> Cutoff Frequency.
- **Components:** `juce::dsp::StateVariableTPTFilter`, `juce::dsp::Oscillator`.

#### D. Vintage / Lo-Fi (Degradation)
- **Wow/Flutter:** Modulated delay line (very short times, <10ms) with multiple LFOs (slow/fast).
- **Noise:** White/Pink noise generator added to signal.
- **Components:** `juce::dsp::DelayLine`, `RandomNoise`.

#### E. Chorus (Modulation)
- **Logic:** Dual/Quad delay lines with phase-offset LFOs.
- **Stereo:** Width control via phase offsets between L/R LFOs.
- **Components:** `juce::dsp::Chorus`.

#### F. Autopan (Spatial)
- **Logic:** LFO modulation of L/R gain.
- **Components:** `juce::dsp::Panner`, `juce::dsp::Oscillator`.

#### G. Delay (Time)
- **Logic:** Stereo feedback delay.
- **Features:** Ping-pong mode (optional), Sync.
- **Components:** `juce::dsp::DelayLine` (circular buffer).

#### H. Reverb (Space)
- **Logic:** Algorithmic reverberation.
- **Components:** `juce::dsp::Reverb` or custom FDN (Feedback Delay Network) for "premium" sound.

## Processing Chain
```
Input (Stereo)
  ⬇
[Halftime] (Resampling/Buffer)
  ⬇
[Saturation] (Oversampled Waveshaper)
  ⬇
[AutoFilter] (SVF + LFO)
  ⬇
[Vintage] (Modulated Delay + Noise)
  ⬇
[Chorus] (Multi-tap Mod Delay)
  ⬇
[Autopan] (Gain Mod)
  ⬇
[Delay] (Feedback Line)
  ⬇
[Reverb] (Diffusion)
  ⬇
Global Mix (Dry/Wet)
  ⬇
Output Gain
  ⬇
Output
```

## Parameter Mapping
| Parameter | Component | Function | Range |
|-----------|-----------|----------|-------|
| `halftime_mix` | Halftime | Wet/Dry blend of slowed signal | 0-100% |
| `sat_drive` | Saturation | Input gain into waveshaper | 0-48dB |
| `filter_cutoff` | AutoFilter | Filter center frequency | 20Hz-20kHz |
| `vintage_wow` | Vintage | LFO depth for delay modulation | 0-1% |
| `chorus_rate` | Chorus | LFO frequency | 0-10Hz |
| `pan_rate` | Autopan | LFO frequency | 0-20Hz |
| `delay_time` | Delay | Buffer calculation length | 0-2s |
| `reverb_decay` | Reverb | RT60 time | 0.1-10s |

## Complexity Assessment
**Score: 3 (Advanced)**
**Rationale:**
- **Recall/State Management:** Managing 8 distinct effects requires a robust state system.
- **Halftime:** Implementing a clean halftime effect without artifacts requires careful windowing and buffering, which is non-trivial compared to standard DSP.
- **Oversampling:** Saturation needs efficient oversampling to sound "premium".
- **Modulation System:** Multiple LFOs across different modules need to be managed efficiently.
