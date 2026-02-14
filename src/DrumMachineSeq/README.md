# DrumMachineSeq

Contributed by Mateusz Wojt \<mateusz.wojt@outlook.com\>

## Description

This is an advanced drum machine with a built-in sequencer inspired by Mutable Instruments Grids. It features three drum voices (kick, snare, and hi-hat) with automatic pattern generation. The sequencer creates rhythmic patterns that evolve based on the complexity parameter, providing endless variation from simple to intricate beats.

The sequencer runs continuously at 16th note resolution, with patterns that adapt in real-time to the complexity setting. LED 1 provides visual feedback of the current tempo, pulsing in sync with the sequencer's timing, while LED 2 indicates the sequencer's running state.

### Features

- **Three drum voices**: Kick, Snare, and Hi-hat using DaisySP synthesis modules
- **Grids-inspired sequencer**: Automatic pattern generation with adjustable complexity per instrument
- **Tap tempo**: Set BPM by tapping the footswitch in time
- **Real-time pattern evolution**: Patterns change as you adjust the complexity switches
- **Visual feedback**: LED 1 indicates current tempo, LED 2 shows play/stop state
- **Individual drum sound control**: Each drum has dedicated pitch and decay parameters

### Controls

| CONTROL      | DESCRIPTION          | NOTES                                                                                    |
| ------------ | -------------------- | ---------------------------------------------------------------------------------------- |
| KNOB 1       | Kick Pitch           | 50-150 Hz, fundamental frequency of the kick drum                                        |
| KNOB 2       | Hi-hat Tone          | Filter cutoff frequency for hi-hat brightness                                            |
| KNOB 3       | Snare Pitch          | 100-700 Hz, fundamental frequency of the snare                                           |
| KNOB 4       | Kick Decay           | Decay time of the kick drum                                                              |
| KNOB 5       | Hi-hat Decay         | Decay time of the hi-hat                                                                 |
| KNOB 6       | Snare Decay/Snap     | Decay time and snappiness of the snare drum                                              |
| SWITCH 1     | Kick Complexity | **UP** - Simple<br/>**MIDDLE** - Medium <br/>**DOWN** - Complex |
| SWITCH 2     | Hi-hat Complexity     | **UP** - Simple<br/>**MIDDLE** - Medium <br/>**DOWN** - Complex |
| SWITCH 3     | Snare Complexity           | **UP** - Simple<br/>**MIDDLE** - Medium <br/>**DOWN** - Complex   |
| FOOTSWITCH 1        | Tap Tempo        | Tap repeatedly to set BPM (30-300 BPM range)                     |
| FOOTSWITCH 2        | Start/Stop        | Toggle sequencer playback on/off                     |

### Sequencer Operation

The sequencer is inspired by Mutable Instruments Grids and generates patterns automatically:

- **Kick pattern**: Always hits on downbeats (steps 1, 5, 9, 13) with additional hits based on complexity
- **Snare pattern**: Always hits on beats 2 and 4 (steps 5 and 13) with variations based on complexity
- **Hi-hat pattern**: Basic 8th note pattern with 16th note variations based on complexity

Each instrument has independent complexity control via the toggle switches, allowing you to create contrasting rhythmic textures (e.g., simple kick with complex hi-hats).

### Tap Tempo

The tap tempo system provides reliable BPM detection:

- Tap FOOTSWITCH 1 at least twice to set the tempo
- Uses averaging of up to 4 taps for stable readings
- Automatically resets if no tap is received within 2 seconds
- Valid range: 30-300 BPM

### Sound Design

- **Kick**: Uses SyntheticBassDrum module for classic 808-style bass drums
- **Snare**: Uses SyntheticSnareDrum module for synthetic snare sounds with adjustable snappiness
- **Hi-hat**: Custom implementation using filtered white noise with ADSR envelope for metallic hi-hat sounds

### Usage Tips

1. **Set your tempo**: Tap FOOTSWITCH 1 in time to establish the BPM
2. **Start the sequencer**: Press FOOTSWITCH 2 to begin playback
3. **Start simple**: Set all complexity switches to UP for basic patterns
4. **Shape the sounds**: Use knobs 1-6 to dial in the perfect drum tones
5. **Add variation**: Move complexity switches to MIDDLE or DOWN for more intricate patterns
6. **Visual feedback**: Watch LED 1 to see the tempo pulse (brighter on downbeats)

### Pattern Characteristics

- **Low complexity (0-0.3)**: Basic four-on-the-floor kick, backbeat snare, simple 8th-note hi-hats
- **Medium complexity (0.3-0.7)**: Syncopated kicks, ghost notes, 16th-note hi-hat variations
- **High complexity (0.7-1.0)**: Complex polyrhythms, rapid-fire hi-hats, intricate kick and snare patterns

The sequencer continuously runs in 16-step loops, creating patterns that feel natural and musical while maintaining the characteristic Grids-style evolution.
