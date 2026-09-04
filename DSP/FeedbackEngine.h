#pragma once
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Ilya Tolstoukhov

#include "PitchDetector.h"
#include <algorithm>
#include <cmath>

namespace screamback {

enum class Mode : int { DF2 = 0, FreqOut = 1, Natural = 2 };
enum class Voice : int { Auto = 0, Fundamental = 1, Second = 2, Third = 3, Fifth = 4 };

struct Parameters {
  Mode mode = Mode::Natural;
  Voice voice = Voice::Auto;
  double amount = 0.72;
  double onsetMs = 350.0;
  double sensitivity = 0.68;
  bool engaged = false;
};

class FeedbackEngine {
public:
  void Prepare(double sampleRate) {
    mSampleRate = std::max(8000.0, sampleRate);
    mPitch.Prepare(mSampleRate);
    Reset();
  }

  void Reset() {
    mPitch.Reset();
    mPhase = 0.0;
    mLfoPhase = 0.0;
    mCurrentHz = 220.0;
    mLastTrackedHz = 0.0;
    mLockedBaseHz = 0.0;
    mWetEnvelope = 0.0;
    mNoteAge = 0.0;
    mPitchHold = 999.0;
    mWasEngaged = false;
    mLocked = false;
    mResY1 = mResY2 = 0.0;
  }

  double ProcessSample(double input, const Parameters& p) {
    mPitch.Push(input);

    const double sensitivity = std::clamp(p.sensitivity, 0.0, 1.0);
    const double confGate = 0.88 - 0.45 * sensitivity;
    const double rmsGate = 0.018 - 0.0165 * sensitivity;
    const bool pitchGood = mPitch.Confidence() >= confGate && mPitch.Rms() >= rmsGate;

    if (pitchGood) {
      const double candidate = mPitch.PitchHz();
      if (candidate > 0.0) {
        mLastTrackedHz = candidate;
        mPitchHold = 0.0;
      }
    } else {
      mPitchHold += 1.0 / mSampleRate;
    }

    const bool risingEdge = p.engaged && !mWasEngaged;

    // SCREAM is a latch: capture one note and keep singing it until SCREAM is
    // released. This deliberately prevents late low-level pitch-detector errors
    // from making the generated feedback jump by octaves while the guitar note
    // itself is dying away.
    if (risingEdge) {
      mLocked = false;
      mNoteAge = 0.0;
      if (mLastTrackedHz > 0.0 && mPitchHold < 0.35)
        LockPitch(mLastTrackedHz);
    }

    if (p.engaged) {
      if (!mLocked && pitchGood && mLastTrackedHz > 0.0) {
        LockPitch(mLastTrackedHz);
      } else if (mLocked && pitchGood && mLastTrackedHz > 0.0) {
        // Follow normal guitar vibrato and bends, but reject large jumps. To
        // capture a different note, release/retrigger SCREAM (or the MIDI gate).
        const double semitones = std::abs(12.0 * std::log2(mLastTrackedHz / mLockedBaseHz));
        if (semitones < 2.5) {
          const double follow = 1.0 - std::exp(-1.0 / (mSampleRate * 0.030));
          mLockedBaseHz += (mLastTrackedHz - mLockedBaseHz) * follow;
        }
      }

      if (mLocked)
        mNoteAge += 1.0 / mSampleRate;
    }

    const double target = mLocked
      ? mLockedBaseHz * HarmonicMultiplier(p)
      : mCurrentHz;
    const double safeTarget = std::clamp(target, 55.0, mSampleRate * 0.42);

    // A modest glide removes clicks when the user changes harmonic. Natural
    // mode changes more slowly so its evolution feels like feedback finding a
    // new resonance rather than a pitch-switching arpeggiator.
    const double glideSeconds = p.mode == Mode::Natural ? 0.14 : 0.055;
    const double glide = 1.0 - std::exp(-1.0 / (mSampleRate * glideSeconds));
    mCurrentHz += (safeTarget - mCurrentHz) * glide;

    const double onsetSeconds = std::max(0.015, p.onsetMs * 0.001);
    const double attackCoeff = 1.0 - std::exp(-1.0 / (mSampleRate * onsetSeconds));
    const double releaseCoeff = 1.0 - std::exp(-1.0 / (mSampleRate * 0.22));
    const double wanted = (p.engaged && mLocked) ? 1.0 : 0.0;
    mWetEnvelope += (wanted - mWetEnvelope) * (wanted > mWetEnvelope ? attackCoeff : releaseCoeff);

    constexpr double twoPi = 6.283185307179586476925286766559;
    mPhase += twoPi * mCurrentHz / mSampleRate;
    if (mPhase >= twoPi)
      mPhase -= twoPi;
    mLfoPhase += twoPi * 4.7 / mSampleRate;
    if (mLfoPhase >= twoPi)
      mLfoPhase -= twoPi;

    const double osc = OscillatorForMode(p.mode);
    const double resonant = Resonator(input, mCurrentHz, p.mode);
    double wet = 0.0;
    switch (p.mode) {
      case Mode::DF2: wet = 0.94 * osc + 0.06 * resonant; break;
      case Mode::FreqOut: wet = 0.68 * osc + 0.32 * resonant; break;
      case Mode::Natural: wet = 0.54 * osc + 0.46 * resonant; break;
    }

    const double amount = std::clamp(p.amount, 0.0, 1.0);
    const double feedback = std::tanh(wet * (1.15 + 1.8 * amount));
    const double output = input + feedback * mWetEnvelope * amount * 0.52;

    mWasEngaged = p.engaged;
    if (!p.engaged && mWetEnvelope < 0.0005) {
      mLocked = false;
      mLockedBaseHz = 0.0;
      mNoteAge = 0.0;
    }

    return output;
  }

  double TrackedPitchHz() const { return mLastTrackedHz; }
  double LockedPitchHz() const { return mLockedBaseHz; }
  double CurrentFeedbackHz() const { return mCurrentHz; }
  double Confidence() const { return mPitch.Confidence(); }
  double Envelope() const { return mWetEnvelope; }
  bool IsLocked() const { return mLocked; }

private:
  void LockPitch(double hz) {
    mLockedBaseHz = std::clamp(hz, 55.0, 1400.0);
    mLocked = true;
    mNoteAge = 0.0;
  }

  double HarmonicMultiplier(const Parameters& p) const {
    if (p.voice != Voice::Auto) {
      switch (p.voice) {
        case Voice::Fundamental: return 1.0;
        case Voice::Second: return 2.0;
        case Voice::Third: return 3.0;
        case Voice::Fifth: return 5.0;
        default: break;
      }
    }

    // Auto is intentionally stable in DF-2 and FreqOut. Only Natural evolves.
    // This makes the first two modes predictable and reserves harmonic motion
    // for the mode whose name promises it.
    switch (p.mode) {
      case Mode::DF2:
        return 1.0;
      case Mode::FreqOut:
        return 2.0;
      case Mode::Natural:
      default:
        if (mNoteAge < 2.4) return 1.0;
        if (mNoteAge < 6.0) return 2.0;
        if (mNoteAge < 10.0) return 3.0;
        return 5.0;
    }
  }

  double OscillatorForMode(Mode mode) const {
    constexpr double centsToRatio = 0.0005776226504666211; // ln(2)/1200
    const double vibratoCents = (mode == Mode::DF2 ? 6.0 : 2.0) * std::sin(mLfoPhase);
    const double phaseWarp = std::exp(centsToRatio * vibratoCents);
    const double s = std::sin(mPhase * phaseWarp);
    if (mode == Mode::Natural)
      return 0.88 * s + 0.12 * std::sin(2.0 * mPhase + 0.17);
    return s;
  }

  double Resonator(double input, double hz, Mode mode) {
    constexpr double twoPi = 6.283185307179586476925286766559;
    const double radius = mode == Mode::Natural ? 0.9972 : 0.9955;
    const double w = twoPi * hz / mSampleRate;
    const double drive = mode == Mode::FreqOut ? 0.035 : 0.024;
    const double y = (1.0 - radius) * input * drive
                   + 2.0 * radius * std::cos(w) * mResY1
                   - radius * radius * mResY2;
    mResY2 = mResY1;
    mResY1 = std::clamp(y, -4.0, 4.0);
    return std::tanh(mResY1 * 12.0);
  }

  PitchDetector mPitch;
  double mSampleRate = 48000.0;
  double mPhase = 0.0;
  double mLfoPhase = 0.0;
  double mCurrentHz = 220.0;
  double mLastTrackedHz = 0.0;
  double mLockedBaseHz = 0.0;
  double mWetEnvelope = 0.0;
  double mNoteAge = 0.0;
  double mPitchHold = 999.0;
  bool mWasEngaged = false;
  bool mLocked = false;
  double mResY1 = 0.0;
  double mResY2 = 0.0;
};

} // namespace screamback
