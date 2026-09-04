// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Ilya Tolstoukhov

#include "DSP/FeedbackEngine.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
  constexpr double sr = 48000.0;
  constexpr double pi = 3.1415926535897932384626433832795;

  screamback::FeedbackEngine engine;
  engine.Prepare(sr);

  screamback::Parameters p;
  p.engaged = true;
  p.mode = screamback::Mode::Natural;
  p.amount = 0.75;
  p.onsetMs = 100.0;
  p.sensitivity = 0.8;

  double energy = 0.0;
  for (int i = 0; i < static_cast<int>(sr * 2.5); ++i) {
    const double t = static_cast<double>(i) / sr;
    const double x = 0.12 * std::sin(2.0 * pi * 220.0 * t) * std::exp(-0.6 * t);
    const double y = engine.ProcessSample(x, p);
    assert(std::isfinite(y));
    energy += std::abs(y);
  }

  std::cout << "tracked=" << engine.TrackedPitchHz()
            << " confidence=" << engine.Confidence()
            << " energy=" << energy << '\n';

  assert(engine.TrackedPitchHz() > 210.0 && engine.TrackedPitchHz() < 230.0);
  assert(energy > 10.0);
  return 0;
}
