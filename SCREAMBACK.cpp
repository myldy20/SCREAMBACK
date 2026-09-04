// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Ilya Tolstoukhov

#include "SCREAMBACK.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

SCREAMBACK::SCREAMBACK(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kMode)->InitEnum("Mode", 2, {"DF-2", "FreqOut", "Natural"});
  GetParam(kVoice)->InitEnum("Voice", 0, {"Auto", "Fundamental", "2nd", "3rd", "5th"});
  GetParam(kAmount)->InitDouble("Amount", 72.0, 0.0, 100.0, 0.1, "%");
  GetParam(kOnset)->InitDouble("Onset", 420.0, 0.0, 2000.0, 1.0, "ms");
  GetParam(kSensitivity)->InitDouble("Sensitivity", 62.0, 0.0, 100.0, 0.1, "%");
  GetParam(kEngage)->InitBool("Engage", false, "", 0, "", "OFF", "ON");

  MakePreset("Natural", 2.0, 0.0, 72.0, 420.0, 62.0, 0.0);
  MakePreset("DF-2 Classic", 0.0, 1.0, 58.0, 650.0, 58.0, 0.0);
  MakePreset("FreqOut Fast", 1.0, 2.0, 78.0, 180.0, 70.0, 0.0);
  MakePreset("Long Howl", 2.0, 0.0, 86.0, 950.0, 68.0, 0.0);

#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS,
                        GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {
    const IColor background(255, 18, 17, 20);
    const IColor panel(255, 31, 29, 34);
    const IColor accent(255, 238, 76, 70);
    const IColor text(255, 236, 232, 226);
    const IColor muted(255, 145, 139, 139);

    const IVStyle style = DEFAULT_STYLE
      .WithColor(kBG, panel)
      .WithColor(kFG, IColor(255, 71, 67, 74))
      .WithColor(kPR, accent)
      .WithLabelText(DEFAULT_LABEL_TEXT.WithFGColor(muted))
      .WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(text))
      .WithRoundness(0.18f)
      .WithFrameThickness(1.5f)
      .WithDrawShadows(false);

    const IVStyle engageStyle = style
      .WithColor(kBG, IColor(255, 38, 35, 39))
      .WithColor(kFG, accent)
      .WithColor(kPR, IColor(255, 255, 116, 91))
      .WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(text).WithSize(22.f));

    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(background);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

    pGraphics->AttachControl(new ITextControl(
      IRECT(30.f, 18.f, 690.f, 68.f), "SCREAMBACK",
      IText(42.f, accent, "Roboto-Regular")));
    pGraphics->AttachControl(new ITextControl(
      IRECT(33.f, 60.f, 690.f, 84.f), "controlled feedback without the amp",
      IText(15.f, muted, "Roboto-Regular")));

    pGraphics->AttachControl(new IVRadioButtonControl(
      IRECT(30.f, 98.f, 690.f, 155.f), kMode,
      {"DF-2", "FREQOUT", "NATURAL"}, "MODE", style,
      EVShape::Rectangle, EDirection::Horizontal, 11.f));

    pGraphics->AttachControl(new IVKnobControl(
      IRECT(30.f, 178.f, 180.f, 300.f), kAmount, "AMOUNT", style));
    pGraphics->AttachControl(new IVKnobControl(
      IRECT(190.f, 178.f, 340.f, 300.f), kOnset, "ONSET", style));
    pGraphics->AttachControl(new IVKnobControl(
      IRECT(350.f, 178.f, 500.f, 300.f), kSensitivity, "SENSE", style));

    pGraphics->AttachControl(new IVMenuButtonControl(
      IRECT(30.f, 316.f, 300.f, 370.f), kVoice, "VOICE", style));

    pGraphics->AttachControl(new IVToggleControl(
      IRECT(510.f, 186.f, 690.f, 302.f), kEngage, "",
      engageStyle, "ARM", "SCREAM"));

    pGraphics->AttachControl(new ITextControl(
      IRECT(30.f, 384.f, 690.f, 405.f),
      "MIDI  C3/CC64 gate   CC20 engage   21 amount   22 onset   23 voice   24 sense   25 mode",
      IText(11.f, muted, "Roboto-Regular")));
    pGraphics->AttachControl(new ITextControl(
      IRECT(30.f, 406.f, 690.f, 424.f),
      "SCREAMBACK by Ilya Tolstoukhov  ·  GPLv3",
      IText(10.f, IColor(255, 94, 89, 92), "Roboto-Regular")));
  };
#endif
}

#if IPLUG_DSP
void SCREAMBACK::OnReset()
{
  mEngine.Prepare(GetSampleRate());
  mMidiGate.store(false);
}

void SCREAMBACK::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const int nIn = NInChansConnected();
  const int nOut = NOutChansConnected();
  if (nOut <= 0)
    return;

  screamback::Parameters p;
  p.mode = static_cast<screamback::Mode>(GetParam(kMode)->Int());
  p.voice = static_cast<screamback::Voice>(GetParam(kVoice)->Int());
  p.amount = GetParam(kAmount)->Value() / 100.0;
  p.onsetMs = GetParam(kOnset)->Value();
  p.sensitivity = GetParam(kSensitivity)->Value() / 100.0;
  p.engaged = GetParam(kEngage)->Bool() || mMidiGate.load();

  for (int s = 0; s < nFrames; ++s) {
    double mono = 0.0;
    if (nIn > 0) {
      for (int c = 0; c < nIn; ++c)
        mono += static_cast<double>(inputs[c][s]);
      mono /= static_cast<double>(nIn);
    }

    const double processedMono = mEngine.ProcessSample(mono, p);
    const double feedbackOnly = processedMono - mono;

    for (int c = 0; c < nOut; ++c) {
      const double dry = nIn > 0
        ? static_cast<double>(inputs[std::min(c, nIn - 1)][s])
        : 0.0;
      // Keep the dry path bit-transparent. Any limiting/saturation belongs to the
      // generated feedback voice inside FeedbackEngine, not to the guitar input.
      outputs[c][s] = static_cast<sample>(dry + feedbackOnly);
    }
  }
}

void SCREAMBACK::SetParamFromMidi(int paramIdx, double normalized, int sampleOffset)
{
  normalized = std::clamp(normalized, 0.0, 1.0);
  GetParam(paramIdx)->SetNormalized(normalized);
  OnParamChange(paramIdx, EParamSource::kHost, sampleOffset);
}

void SCREAMBACK::ProcessMidiMsg(const IMidiMsg& msg)
{
  if (msg.StatusMsg() == IMidiMsg::kNoteOn && msg.NoteNumber() == 60) {
    mMidiGate.store(msg.Velocity() > 0);
    return;
  }

  if (msg.StatusMsg() == IMidiMsg::kNoteOff && msg.NoteNumber() == 60) {
    mMidiGate.store(false);
    return;
  }

  if (msg.StatusMsg() != IMidiMsg::kControlChange)
    return;

  const int cc = static_cast<int>(msg.ControlChangeIdx());
  const double value = msg.ControlChange(msg.ControlChangeIdx());

  switch (cc) {
    case 20: SetParamFromMidi(kEngage, value >= 0.5 ? 1.0 : 0.0, msg.mOffset); break;
    case 21: SetParamFromMidi(kAmount, value, msg.mOffset); break;
    case 22: SetParamFromMidi(kOnset, value, msg.mOffset); break;
    case 23: SetParamFromMidi(kVoice, value, msg.mOffset); break;
    case 24: SetParamFromMidi(kSensitivity, value, msg.mOffset); break;
    case 25: SetParamFromMidi(kMode, value, msg.mOffset); break;
    case 64: mMidiGate.store(value >= 0.5); break;
    default: break;
  }
}
#endif
