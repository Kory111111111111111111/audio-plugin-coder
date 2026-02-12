# WAVFin Effect Engine - Parameter Spec

## Global
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `global_mix` | Mix | Float | 0.0 - 1.0 | 1.0 | % | Global wet/dry mix |
| `output_gain` | Output | Float | -24.0 - 24.0 | 0.0 | dB | Master output volume |

## Reverb
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `reverb_enable` | Enable | Bool | 0 - 1 | 0 | - | Toggle Reverb |
| `reverb_size` | Size | Float | 0.0 - 1.0 | 0.5 | % | Room size |
| `reverb_decay` | Decay | Float | 0.1 - 10.0 | 2.0 | s | Decay time |
| `reverb_mix` | Mix | Float | 0.0 - 1.0 | 0.3 | % | Reverb wet level |

## Delay
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `delay_enable` | Enable | Bool | 0 - 1 | 0 | - | Toggle Delay |
| `delay_time` | Time | Float | 0.0 - 2.0 | 0.5 | s | Delay time (sync optional) |
| `delay_feedback` | Feedback | Float | 0.0 - 1.0 | 0.4 | % | Feedback amount |
| `delay_mix` | Mix | Float | 0.0 - 1.0 | 0.3 | % | Delay wet level |

## Chorus
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `chorus_enable` | Enable | Bool | 0 - 1 | 0 | - | Toggle Chorus |
| `chorus_rate` | Rate | Float | 0.0 - 10.0 | 1.0 | Hz | Modulation rate |
| `chorus_depth` | Depth | Float | 0.0 - 1.0 | 0.5 | % | Modulation depth |
| `chorus_mix` | Mix | Float | 0.0 - 1.0 | 0.5 | % | Chorus wet level |

## AutoFilter
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `filter_enable` | Enable | Bool | 0 - 1 | 0 | - | Toggle Filter |
| `filter_cutoff` | Cutoff | Float | 20.0 - 20000.0 | 2000.0 | Hz | Filter cutoff frequency |
| `filter_res` | Resonance | Float | 0.0 - 1.0 | 0.1 | - | Filter resonance |
| `filter_lfo_rate` | LFO Rate | Float | 0.0 - 20.0 | 2.0 | Hz | Cutoff modulation rate |
| `filter_lfo_depth` | LFO Depth | Float | 0.0 - 1.0 | 0.0 | % | Cutoff modulation depth |

## Autopan
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `pan_enable` | Enable | Bool | 0 - 1 | 0 | - | Toggle Autopan |
| `pan_rate` | Rate | Float | 0.0 - 20.0 | 1.0 | Hz | Panning speed |
| `pan_depth` | Depth | Float | 0.0 - 1.0 | 1.0 | % | Panning width |

## Halftime
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `halftime_enable` | Enable | Bool | 0 - 1 | 0 | - | Toggle Halftime |
| `halftime_mix` | Mix | Float | 0.0 - 1.0 | 1.0 | % | Blend between raw and slowed signal |
| `halftime_fade` | Smooth | Float | 0.0 - 100.0 | 10.0 | ms | Crossfade smoothing for clean loops |

## Vintage (Lo-Fi)
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `vintage_enable` | Enable | Bool | 0 - 1 | 0 | - | Toggle Vintage |
| `vintage_wow` | Wow | Float | 0.0 - 1.0 | 0.2 | % | Slow pitch modulation |
| `vintage_flutter` | Flutter | Float | 0.0 - 1.0 | 0.2 | % | Fast pitch modulation |
| `vintage_noise` | Noise | Float | 0.0 - 1.0 | 0.0 | % | Background noise level |

## Saturation
| ID | Name | Type | Range | Default | Unit | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `sat_enable` | Enable | Bool | 0 - 1 | 0 | - | Toggle Saturation |
| `sat_drive` | Drive | Float | 0.0 - 48.0 | 0.0 | dB | Input drive |
| `sat_type` | Type | Choice | 0 - 3 | 0 | - | 0:Tube, 1:Tape, 2:Diode, 3:Digital |
| `sat_mix` | Mix | Float | 0.0 - 1.0 | 1.0 | % | Saturation blend |
