// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Ilya Tolstoukhov

#include "DSP/FeedbackEngine.h"
#include <cassert>
#include <cmath>
#include <iostream>

namespace {
constexpr double kSampleRate = 48000.0;
constexpr double kPi = 3.1415926535897932384626433832795;

double GuitarTone(int sample, double seconds, double decay = 0.0)
{
  const double t = static_cast<double>(sample) / kSampleRate;
  if (t >= seconds)
    return 0.0;
  const double env = decay > 0.0 ? std::exp(-decay * t) : 1.0;
  return 0.12 * std::sin(2.0 * kPi * 220.0 * t) * env;
}
}

int main()
{
  // A fixed harmonic must latch the detected note and keep singing after the
  // guitar itself has decayed to silence. This is the core pedal behaviour.
  screamback::FeedbackEngine engine;
  engine.Prepare(kSampleRate);

  screamback::Parameters p;
  p.engaged = true;
  p.mode = screamback::Mode::FreqOut;
  p.voice = screamback::Voice::Second;
  p.amount = 0.75;
  p.onsetMs = 100.0;
  p.sensitivity = 0.8;

  double lateWetEnergy = 0.0;
  const int totalSamples = static_cast<int>(kSampleRate * 5.0);
  for (int i = 0; i < totalSamples; ++i) {
    const double x = GuitarTone(i, 0.9, 0.7);
    const double y = engine.ProcessSample(x, p);
    assert(std::isfinite(y));
    if (i > static_cast<int>(kSampleRate * 3.0))
      lateWetEnergy += std::abs(y);
  }

  std::cout << "tracked=" << engine.TrackedPitchHz()
            << " locked=" << engine.LockedPitchHz()
            << " feedback=" << engine.CurrentFeedbackHz()
            << " envelope=" << engine.Envelope()
            << " lateEnergy=" << lateWetEnergy << '\n';

  assert(engine.TrackedPitchHz() > 210.0 && engine.TrackedPitchHz() < 230.0);
  assert(engine.IsLocked());
  assert(engine.LockedPitchHz() > 210.0 && engine.LockedPitchHz() < 230.0);
  assert(engine.CurrentFeedbackHz() > 420.0 && engine.CurrentFeedbackHz() < 460.0);
  assert(engine.Envelope() > 0.95);
  assert(lateWetEnergy > 100.0);

  // Releasing SCREAM should release the generated voice and eventually unlock
  // it rather than leaving hidden state behind for the next note.
  p.engaged = false;
  for (int i = 0; i < static_cast<int>(kSampleRate * 2.0); ++i)
    engine.ProcessSample(0.0, p);
  assert(engine.Envelope() < 0.001);
  assert(!engine.IsLocked());

  // RISE must be audibly meaningful: after the same 0.5 s note, a fast rise
  // should have built substantially more feedback envelope than a slow one.
  screamback::FeedbackEngine fast;
  screamback::FeedbackEngine slow;
  fast.Prepare(kSampleRate);
  slow.Prepare(kSampleRate);

  screamback::Parameters fastP = p;
  fastP.engaged = true;
  fastP.mode = screamback::Mode::DF2;
  fastP.voice = screamback::Voice::Fundamental;
  fastP.onsetMs = 50.0;

  screamback::Parameters slowP = fastP;
  slowP.onsetMs = 1000.0;

  for (int i = 0; i < static_cast<int>(kSampleRate * 0.5); ++i) {
    const double x = GuitarTone(i, 0.5);
    fast.ProcessSample(x, fastP);
    slow.ProcessSample(x, slowP);
  }

  std::cout << "fastRise=" << fast.Envelope()
            << " slowRise=" << slow.Envelope() << '\n';
  assert(fast.Envelope() > 0.95);
  assert(slow.Envelope() < 0.45);

  return 0;
}
