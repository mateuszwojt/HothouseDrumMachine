// DrumMachineSeq for Hothouse DIY DSP Platform
// Copyright (C) 2024 Mateusz Wojt <mateusz.wojt@outlook.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "daisysp.h"
#include "hothouse.h"

using clevelandmusicco::Hothouse;
using daisy::AudioHandle;
using daisy::Led;
using daisy::SaiHandle;
using daisysp::SyntheticBassDrum;
using daisysp::SyntheticSnareDrum;

// Hi-hat implementation using noise and envelope
class HiHat
{
public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        noise_.Init();
        filter_.Init(sample_rate);
        filter_.SetRes(0.2f);
        envelope_.Init(sample_rate);
        envelope_.SetTime(daisysp::ADSR_SEG_ATTACK, 0.001f);
        envelope_.SetTime(daisysp::ADSR_SEG_DECAY, 0.05f);
        envelope_.SetTime(daisysp::ADSR_SEG_RELEASE, 0.01f);
        envelope_.SetSustainLevel(0.0f);
    }
    
    void SetFreq(float freq) // 0-1
    {
        // Use a high-pass filter for the hi-hat "pitch"
        float cutoff = 2000.0f + freq * 8000.0f;
        filter_.SetFreq(cutoff);
    }
    
    void SetDecay(float decay)
    {
        // Shorter decay range: 0.005s to 0.15s
        envelope_.SetTime(daisysp::ADSR_SEG_DECAY, 0.005f + decay * 0.145f);
    }
    
    void Trig()
    {
        envelope_.Retrigger(true);
    }
    
    float Process()
    {
        float noise = noise_.Process() * 0.5f;
        filter_.Process(noise);
        float filtered = filter_.High();
        return filtered * envelope_.Process(true);
    }
    
private:
    float sample_rate_;
    daisysp::WhiteNoise noise_;
    daisysp::Svf filter_;
    daisysp::Adsr envelope_;
};

// Sequencer class inspired by Mutable Instruments Grids
class GridsSequencer
{
public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        SetTempo(tempo_);
        Reset();
        
        // Initialize some default patterns
        GeneratePatterns();
    }
    
    void SetTempo(float bpm)
    {
        tempo_ = bpm;
        step_time_ = 60.0f / (bpm * 4.0f); // 16th notes
    }
    
    float GetTempo() { return tempo_; }

    void SetComplexity(int instrument, float complexity)
    {
        // Only regenerate if change is significant
        if (fabsf(complexity_[instrument] - complexity) > 0.05f)
        {
            complexity_[instrument] = complexity;
            GeneratePatterns();
        }
    }
    
    void SetRunning(bool running) { running_ = running; }
    bool IsRunning() { return running_; }

    void Reset()
    {
        current_step_ = 0;
        phase_ = 0.0f;
    }
    
    void Process(float dt)
    {
        if (!running_) return;

        phase_ += dt;
        
        if (phase_ >= step_time_)
        {
            phase_ -= step_time_;
            current_step_ = (current_step_ + 1) % 16;
        }
    }
    
    bool GetKick()
    {
        return running_ && kick_pattern_[current_step_];
    }
    
    bool GetSnare()
    {
        return running_ && snare_pattern_[current_step_];
    }
    
    bool GetHiHat()
    {
        return running_ && hihat_pattern_[current_step_];
    }
    
    int GetCurrentStep()
    {
        return current_step_;
    }
    
    float GetPhase()
    {
        return phase_ / step_time_;
    }
    
private:
    void GeneratePatterns()
    {
        // Generate kick pattern
        for (int i = 0; i < 16; i++)
        {
            if (i % 4 == 0) // Always on downbeats
            {
                kick_pattern_[i] = true;
            }
            else
            {
                kick_pattern_[i] = (rand() % 100) < (complexity_[0] * 30);
            }
        }
        
        // Generate snare pattern
        for (int i = 0; i < 16; i++)
        {
            if (i % 8 == 4) // Always on 2 and 4
            {
                snare_pattern_[i] = true;
            }
            else if (i % 4 == 2) // Sometimes on off-beats
            {
                snare_pattern_[i] = (rand() % 100) < (complexity_[2] * 50);
            }
            else
            {
                snare_pattern_[i] = (rand() % 100) < (complexity_[2] * 20);
            }
        }
        
        // Generate hihat pattern
        for (int i = 0; i < 16; i++)
        {
            if (i % 2 == 0) // Basic 8th note pattern
            {
                hihat_pattern_[i] = true;
            }
            else // 16th notes based on complexity
            {
                hihat_pattern_[i] = (rand() % 100) < (complexity_[1] * 70);
            }
        }
    }
    
    float sample_rate_;
    float tempo_ = 120.0f;
    float step_time_;
    float phase_ = 0.0f;
    float complexity_[3] = {0.5f, 0.5f, 0.5f}; // Kick, Hihat, Snare
    int current_step_ = 0;
    bool running_ = true;
    
    bool kick_pattern_[16];
    bool snare_pattern_[16];
    bool hihat_pattern_[16];
};

class TapTempo
{
public:
    static constexpr int kNumTaps = 4;
    static constexpr uint32_t kTimeout = 2000;      // Reset after 2s
    static constexpr uint32_t kMinInterval = 200;   // 300 BPM max
    static constexpr uint32_t kMaxInterval = 2000;  // 30 BPM min
    static constexpr uint32_t kDebounceTime = 50;   // 50ms debounce
    
    void Init(float default_bpm = 120.0f)
    {
        tempo_ = default_bpm;
        Reset();
    }
    
    void Reset()
    {
        tap_count_ = 0;
        last_tap_time_ = 0;
    }
    
    // Returns true if tempo was updated
    bool Tap()
    {
        uint32_t now = daisy::System::GetNow();
        
        // Debounce: ignore taps too close together
        if (last_tap_time_ != 0 && (now - last_tap_time_) < kDebounceTime)
        {
            return false;
        }
        
        // First tap or timeout: just record time and reset
        if (last_tap_time_ == 0 || (now - last_tap_time_) > kTimeout)
        {
            Reset();
            last_tap_time_ = now;
            return false;
        }
        
        uint32_t interval = now - last_tap_time_;
        last_tap_time_ = now;
        
        // Reject intervals outside valid BPM range
        if (interval < kMinInterval || interval > kMaxInterval)
        {
            Reset();
            last_tap_time_ = now;
            return false;
        }
        
        // Shift history and add new interval
        for (int i = kNumTaps - 1; i > 0; i--)
        {
            intervals_[i] = intervals_[i - 1];
        }
        intervals_[0] = interval;
        
        if (tap_count_ < kNumTaps)
        {
            tap_count_++;
        }
        
        // Calculate weighted average (recent taps weighted more)
        float weighted_sum = 0.0f;
        float weight_total = 0.0f;
        for (int i = 0; i < tap_count_; i++)
        {
            float weight = static_cast<float>(tap_count_ - i);
            weighted_sum += intervals_[i] * weight;
            weight_total += weight;
        }
        
        tempo_ = 60000.0f / (weighted_sum / weight_total);
        return true;
    }
    
    float GetTempo() const { return tempo_; }
    
private:
    uint32_t intervals_[kNumTaps] = {0};
    uint32_t last_tap_time_ = 0;
    int tap_count_ = 0;
    float tempo_ = 120.0f;
};

Led ledBpm;
Led ledStatus;
Hothouse hw;

SyntheticBassDrum kick;
SyntheticSnareDrum snare;
HiHat hihat;

GridsSequencer sequencer;
TapTempo tapTempo;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    hw.ProcessAllControls();

    // Tap Tempo on Footswitch 1
    if (hw.switches[Hothouse::FOOTSWITCH_1].FallingEdge())
    {
        if (tapTempo.Tap())
        {
            sequencer.SetTempo(tapTempo.GetTempo());
        }
    }

    // Start/Stop on Footswitch 2
    if (hw.switches[Hothouse::FOOTSWITCH_2].FallingEdge())
    {
        sequencer.SetRunning(!sequencer.IsRunning());
    }

    // Instrument complexity from switches
    const float complexities[] = {0.1f, 0.5f, 0.9f};
    sequencer.SetComplexity(0, complexities[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)]); // Kick
    sequencer.SetComplexity(1, complexities[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2)]); // Hihat
    sequencer.SetComplexity(2, complexities[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3)]); // Snare

    // Drum sound parameters (Knobs 1-6)
    // Kick: Knob 1 (Pitch), Knob 4 (Decay)
    kick.SetFreq(50.0f + hw.knobs[Hothouse::KNOB_1].Process() * 150.0f);
    kick.SetDecay(hw.knobs[Hothouse::KNOB_4].Process());
    
    // Hihat: Knob 2 (Pitch/Tone proxy), Knob 5 (Decay)
    hihat.SetFreq(hw.knobs[Hothouse::KNOB_2].Process());
    hihat.SetDecay(hw.knobs[Hothouse::KNOB_5].Process());

    // Snare: Knob 3 (Pitch), Knob 6 (Decay + Snap)
    snare.SetFreq(100.0f + hw.knobs[Hothouse::KNOB_3].Process() * 600.0f);
    snare.SetDecay(hw.knobs[Hothouse::KNOB_6].Process());
    snare.SetSnappy(hw.knobs[Hothouse::KNOB_6].Process());

    // Process sequencer
    float dt_per_sample = 1.0f / hw.AudioSampleRate();
    int last_step = sequencer.GetCurrentStep();
    
    for (size_t i = 0; i < size; i++)
    {
        sequencer.Process(dt_per_sample);
        
        int current_step = sequencer.GetCurrentStep();
        if (current_step != last_step)
        {
            // Step changed - check for triggers
            if (sequencer.GetKick()) kick.Trig();
            if (sequencer.GetSnare()) snare.Trig();
            if (sequencer.GetHiHat()) hihat.Trig();
            last_step = current_step;
        }
        
        float sig = (kick.Process() + snare.Process() + hihat.Process()) * 0.5f;
        out[0][i] = out[1][i] = sig;
    }

    // LED updates
    float phase = sequencer.GetPhase();
    ledBpm.Set((sequencer.GetCurrentStep() % 4 == 0) ? (1.0f - phase) : (0.3f * (1.0f - phase)));
    ledBpm.Update();
    ledStatus.Set(sequencer.IsRunning() ? 1.0f : 0.0f);
    ledStatus.Update();
}

int main()
{
    hw.Init();
    hw.SetAudioBlockSize(4);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    kick.Init(hw.AudioSampleRate());
    snare.Init(hw.AudioSampleRate());
    hihat.Init(hw.AudioSampleRate());

    tapTempo.Init(120.0f);
    sequencer.Init(hw.AudioSampleRate());

    ledBpm.Init(hw.seed.GetPin(Hothouse::LED_1), false);
    ledStatus.Init(hw.seed.GetPin(Hothouse::LED_2), false);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while (true)
    {
        hw.DelayMs(6);
        hw.CheckResetToBootloader();
    }
    return 0;
}
