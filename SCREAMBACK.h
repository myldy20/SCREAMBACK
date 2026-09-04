#pragma once
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Ilya Tolstoukhov

#include "IPlug_include_in_plug_hdr.h"
#include "DSP/FeedbackEngine.h"
#include <atomic>

const int kNumPresets = 4;

enum EParams {
  kMode = 0,
  kVoice,
  kAmount,
  kOnset,
  kSensitivity,
  kEngage,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class SCREAMBACK final : public Plugin {
public:
  SCREAMBACK(const InstanceInfo& info);

#if IPLUG_DSP
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void OnReset() override;
#endif

private:
#if IPLUG_DSP
  void SetParamFromMidi(int paramIdx, double normalized, int sampleOffset);

  screamback::FeedbackEngine mEngine;
  std::atomic<bool> mMidiGate {false};
#endif
};
