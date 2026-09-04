// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Ilya Tolstoukhov

#include "SCREAMBACK.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

SCREAMBACK::SCREAMBACK(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  // The default should do the obvious thing: focused octave feedback, quickly.
  // Natural/Auto is intentionally an opt-in personality because it evolves.
  GetParam(kMode)->InitEnum("Mode", 1, {"DF-2", "FreqOut", "Natural"});
  GetParam(kVoice)->InitEnum("Harmonic", 2, {"Auto", "Fundamental", "2nd", "3rd", "5th"});
  GetParam(kAmount)->InitDouble("Level", 72.0, 0.0, 100.0, 0.1, "%");
  GetParam(kOnset)->InitDouble("Rise", 250.0, 15.0, 1500.0, 1.0, "ms");
  GetParam(kSensitivity)->InitDouble("Track", 70.0, 0.0, 100.0, 0.1, "%");
  GetParam(kEngage)->InitBool("Scream", false, "", 0, "", "OFF", "ON");

  MakePreset("FreqOut / 2nd", 1.0, 2.0, 72.0, 250.0, 70.0, 0.0);
  MakePreset("DF-2 Classic", 0.0, 1.0, 60.0, 550.0, 62.0, 0.0);
  MakePreset("Natural Evolve", 2.0, 0.0, 74.0, 500.0, 70.0, 0.0);
  MakePreset("Long Howl", 2.0, 0.0, 86.0, 1000.0, 74.0, 0.0);

#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS,
                        GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {
    const IColor background(255, 14, 14, 17);
    const IColor panel(255, 30, 29, 34);
    const IColor panelRaised(255, 39, 37, 43);
    const IColor accent(255, 255, 82, 74);
    const IColor accentHot(255, 255, 112, 91);
    const IColor text(255, 244, 242, 239);
    const IColor muted(255, 176, 171, 174);
    const IColor dim(255, 119, 114, 120);
    const IColor control(255, 105, 101, 111);

    const IVStyle knobStyle = DEFAULT_STYLE
      .WithColor(kBG, panel)
      .WithColor(kFG, control)
      .WithColor(kPR, accent)
      .WithLabelText(DEFAULT_LABEL_TEXT.WithFGColor(text).WithSize(15.f))
      .WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(text).WithSize(14.f))
      .WithRoundness(0.18f)
      .WithFrameThickness(1.5f)
      .WithDrawShadows(false);

    const IVStyle modeStyle = DEFAULT_STYLE
      .WithColor(kBG, panelRaised)
      .WithColor(kFG, text)
      .WithColor(kPR, accent)
      .WithLabelText(DEFAULT_LABEL_TEXT.WithFGColor(text).WithSize(14.f))
      .WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(text).WithSize(13.f))
      .WithRoundness(0.14f)
      .WithFrameThickness(1.5f)
      .WithDrawShadows(false);

    const IVStyle menuStyle = knobStyle
      .WithColor(kBG, panelRaised)
      .WithColor(kFG, text);

    const IVStyle engageStyle = DEFAULT_STYLE
      .WithColor(kBG, panelRaised)
      .WithColor(kFG, accent)
      .WithColor(kPR, accentHot)
      .WithLabelText(DEFAULT_LABEL_TEXT.WithFGColor(text).WithSize(14.f))
      .WithValueText(DEFAULT_VALUE_TEXT.WithFGColor(text).WithSize(20.f))
      .WithRoundness(0.16f)
      .WithFrameThickness(2.f)
      .WithDrawShadows(false);

    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(background);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

    pGraphics->AttachControl(new ITextControl(
      IRECT(30.f, 14.f, 690.f, 58.f), "SCREAMBACK",
      IText(38.f, accent, "Roboto-Regular")));
    pGraphics->AttachControl(new ITextControl(
      IRECT(30.f, 54.f, 690.f, 76.f), "controlled feedback without the amp",
      IText(14.f, muted, "Roboto-Regular")));

    pGraphics->AttachControl(new ITextControl(
      IRECT(45.f, 82.f, 675.f, 104.f),
      "PLAY A NOTE  →  PRESS SCREAM  →  FEEDBACK HOLDS UNTIL YOU STOP IT",
      IText(11.f, text, "Roboto-Regular")));

    pGraphics->AttachControl(new IVRadioButtonControl(
      IRECT(30.f, 112.f, 690.f, 164.f), kMode,
      {"DF-2", "FREQOUT", "NATURAL"}, "MODE", modeStyle,
      EVShape::Rectangle, EDirection::Horizontal, 11.f));
    pGraphics->AttachControl(new ITextControl(
      IRECT(40.f, 164.f, 680.f, 184.f),
      "synthetic / old-school     •     focused / stable     •     evolving harmonics",
      IText(10.f, dim, "Roboto-Regular")));

    pGraphics->AttachControl(new IVKnobControl(
      IRECT(30.f, 194.f, 178.f, 306.f), kAmount, "LEVEL", knobStyle));
    pGraphics->AttachControl(new IVKnobControl(
      IRECT(190.f, 194.f, 338.f, 306.f), kOnset, "RISE", knobStyle));
    pGraphics->AttachControl(new IVKnobControl(
      IRECT(350.f, 194.f, 498.f, 306.f), kSensitivity, "TRACK", knobStyle));

    pGraphics->AttachControl(new ITextControl(
      IRECT(30.f, 301.f, 178.f, 321.f), "feedback volume",
      IText(10.f, muted, "Roboto-Regular")));
    pGraphics->AttachControl(new ITextControl(
      IRECT(190.f, 301.f, 338.f, 321.f), "time to bloom",
      IText(10.f, muted, "Roboto-Regular")));
    pGraphics->AttachControl(new ITextControl(
      IRECT(350.f, 301.f, 498.f, 321.f), "how easily a note locks",
      IText(10.f, muted, "Roboto-Regular")));

    pGraphics->AttachControl(new IVToggleControl(
      IRECT(510.f, 200.f, 690.f, 309.f), kEngage, "FEEDBACK",
      engageStyle, "SCREAM", "SCREAMING"));
    pGraphics->AttachControl(new ITextControl(
      IRECT(510.f, 307.f, 690.f, 325.f), "click = latch   •   MIDI = hold",
      IText(9.f, muted, "Roboto-Regular")));

    pGraphics->AttachControl(new IVMenuButtonControl(
      IRECT(30.f, 334.f, 390.f, 388.f), kVoice, "HARMONIC", menuStyle));
    pGraphics->AttachControl(new ITextControl(
      IRECT(400.f, 339.f, 690.f, 361.f),
      "Auto is stable in DF-2/FreqOut;\nonly Natural evolves",
      IText(9.f, muted, "Roboto-Regular")));

    pGraphics->AttachControl(new ITextControl(
      IRECT(30.f, 402.f, 690.f, 421.f),
      "MIDI: C3 or CC64 = momentary SCREAM   •   CC20–25 = plug-in controls",
      IText(10.f, muted, "Roboto-Regular")));
    pGraphics->AttachControl(new ITextControl(
      IRECT(30.f, 428.f, 690.f, 445.f),
      "SCREAMBACK v0.2  ·  Ilya Tolstoukhov  ·  GPLv3",
      IText(9.f, dim, "Roboto-Regular")));
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
