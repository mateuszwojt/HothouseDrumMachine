// DrumMachine for Hothouse DIY DSP Platform
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

// ### Uncomment if IntelliSense can't resolve DaisySP-LGPL classes ###
// #include "daisysp-lgpl.h"

#include "daisysp.h"
#include "hothouse.h"

using clevelandmusicco::Hothouse;
using daisy::AudioHandle;
using daisy::Led;
using daisy::SaiHandle;
using daisysp::Decimator;
using daisysp::SyntheticBassDrum;
using daisysp::SyntheticSnareDrum;
using daisysp::Wavefolder;

Led ledKick, ledSnare;
Hothouse hw;

SyntheticBassDrum kick;
SyntheticSnareDrum snare;

Decimator decimator;
Wavefolder folder;

const float kBitDepth[] = {0.0f, 0.23f, 0.5f};
const float kCrushRate[] = {0.25f, 0.5f, 0.75f};
const float kWavefoldAmount[] = {1.0f, 2.0f, 3.0f};
const float kWavefoldOffset[] = {0.0f, 1.0f, 2.0f};
const float kWavefoldCompensation[] = {1.0f, 0.7f, 0.5f}; // Gain compensation

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
  hw.ProcessAllControls();

  // Process effect controls
  int switch1Pos = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);
  int switch2Pos = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2);
  int switch3Pos = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3);

  decimator.SetDownsampleFactor(kBitDepth[switch1Pos]);
  decimator.SetBitcrushFactor(kCrushRate[switch2Pos]);

  // Footswitch triggers
  if (hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge())
  { // Kick
    kick.Trig();
    ledKick.Set(1.0f);
  }
  else
  {
    ledKick.Set(0.0f);
  }

  if (hw.switches[Hothouse::FOOTSWITCH_2].RisingEdge())
  { // Snare
    snare.Trig();
    ledSnare.Set(1.0f);
  }
  else
  {
    ledSnare.Set(0.0f);
  }

  ledKick.Update();
  ledSnare.Update();

  // Kick parameters
  float kickPitch = 50.0f + hw.knobs[Hothouse::KNOB_1].Process() * 100.0f; // 50-150 Hz
  float kickDecay = hw.knobs[Hothouse::KNOB_4].Process();
  float kickTone = hw.knobs[Hothouse::KNOB_2].Process();
  kick.SetFreq(kickPitch);
  kick.SetDecay(kickDecay);
  kick.SetTone(kickTone);

  // Snare parameters
  float snareFreq = 100.0f + hw.knobs[Hothouse::KNOB_3].Process() * 500.0f; // 100-600 Hz
  float snareDecay = hw.knobs[Hothouse::KNOB_6].Process();
  float snareSnap = hw.knobs[Hothouse::KNOB_5].Process();
  snare.SetFreq(snareFreq);
  snare.SetDecay(snareDecay);
  snare.SetSnappy(snareSnap);

  for (size_t i = 0; i < size; i++)
  {
    float kickSample = kick.Process();
    float snareSample = snare.Process();

    float sig = kickSample + snareSample;
    // Apply decimator effect
    sig = decimator.Process(sig);

    // Apply wavefolder effect with gain compensation
    folder.SetGain(kWavefoldAmount[switch3Pos]);
    folder.SetOffset(kWavefoldOffset[switch3Pos]);
    sig = folder.Process(sig);

    // Apply gain compensation based on wavefold amount
    float outputGain = 0.5f * kWavefoldCompensation[switch3Pos];
    out[0][i] = out[1][i] = sig * outputGain;
  }
}

int main()
{
  hw.Init();
  hw.SetAudioBlockSize(4);
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

  kick.Init(hw.AudioSampleRate());
  snare.Init(hw.AudioSampleRate());
  decimator.Init();
  folder.Init();

  ledKick.Init(hw.seed.GetPin(Hothouse::LED_1), false);  // LED near left footswitch
  ledSnare.Init(hw.seed.GetPin(Hothouse::LED_2), false); // LED near right footswitch

  hw.StartAdc();
  hw.StartAudio(AudioCallback);

  while (true)
  {
    hw.DelayMs(6);
    hw.CheckResetToBootloader();
  }
  return 0;
}