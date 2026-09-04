#pragma once
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Ilya Tolstoukhov

#include <algorithm>
#include <array>
#include <cmath>

namespace screamback {

class PitchDetector {
public:
  void Prepare(double sampleRate) {
    mDecimation = std::clamp(static_cast<int>(std::lround(sampleRate / 12000.0)), 1, 16);
    mAnalysisRate = sampleRate / static_cast<double>(mDecimation);
    Reset();
  }

  void Reset() {
    mBuffer.fill(0.0);
    mDifference.fill(0.0);
    mCmnd.fill(1.0);
    mWrite = 0;
    mFilled = 0;
    mDecimCount = 0;
    mDecimSum = 0.0;
    mSamplesUntilAnalysis = 0;
    mPitchHz = 0.0;
    mConfidence = 0.0;
    mRms = 0.0;
  }

  void Push(double x) {
    mDecimSum += x;
    if (++mDecimCount < mDecimation)
      return;

    const double downsampled = mDecimSum / static_cast<double>(mDecimation);
    mDecimCount = 0;
    mDecimSum = 0.0;

    mBuffer[mWrite] = downsampled;
    mWrite = (mWrite + 1) % kBufferSize;
    mFilled = std::min(mFilled + 1, kBufferSize);

    if (mFilled == kBufferSize && --mSamplesUntilAnalysis <= 0) {
      Analyze();
      mSamplesUntilAnalysis = kAnalysisHop;
    }
  }

  double PitchHz() const { return mPitchHz; }
  double Confidence() const { return mConfidence; }
  double Rms() const { return mRms; }

private:
  static constexpr int kBufferSize = 768;
  static constexpr int kMaxTauStorage = 384;
  static constexpr int kAnalysisHop = 64;

  double SampleAt(int chronologicalIndex) const {
    return mBuffer[(mWrite + chronologicalIndex) % kBufferSize];
  }

  void Analyze() {
    const int minTau = std::max(2, static_cast<int>(mAnalysisRate / 1400.0));
    const int maxTau = std::min(kMaxTauStorage - 2,
                                static_cast<int>(mAnalysisRate / 55.0));
    if (maxTau <= minTau + 2)
      return;

    const int compareLength = kBufferSize - maxTau;
    double mean = 0.0;
    for (int i = 0; i < kBufferSize; ++i)
      mean += SampleAt(i);
    mean /= static_cast<double>(kBufferSize);

    double energy = 0.0;
    for (int i = 0; i < kBufferSize; ++i) {
      const double v = SampleAt(i) - mean;
      energy += v * v;
    }
    mRms = std::sqrt(energy / static_cast<double>(kBufferSize));
    if (mRms < 1.0e-5) {
      mConfidence *= 0.85;
      return;
    }

    mDifference.fill(0.0);
    mCmnd.fill(1.0);
    for (int tau = 1; tau <= maxTau; ++tau) {
      double sum = 0.0;
      for (int i = 0; i < compareLength; ++i) {
        const double d = (SampleAt(i) - mean) - (SampleAt(i + tau) - mean);
        sum += d * d;
      }
      mDifference[tau] = sum;
    }

    double running = 0.0;
    for (int tau = 1; tau <= maxTau; ++tau) {
      running += mDifference[tau];
      mCmnd[tau] = running > 1.0e-18
        ? mDifference[tau] * static_cast<double>(tau) / running
        : 1.0;
    }

    int bestTau = -1;
    constexpr double threshold = 0.16;
    for (int tau = minTau; tau < maxTau; ++tau) {
      if (mCmnd[tau] < threshold && mCmnd[tau] <= mCmnd[tau + 1]) {
        while (tau + 1 < maxTau && mCmnd[tau + 1] < mCmnd[tau])
          ++tau;
        bestTau = tau;
        break;
      }
    }

    if (bestTau < 0) {
      double best = 0.42;
      for (int tau = minTau; tau <= maxTau; ++tau) {
        if (mCmnd[tau] < best) {
          best = mCmnd[tau];
          bestTau = tau;
        }
      }
    }

    if (bestTau < 0) {
      mConfidence *= 0.9;
      return;
    }

    double refinedTau = static_cast<double>(bestTau);
    if (bestTau > minTau && bestTau < maxTau) {
      const double a = mCmnd[bestTau - 1];
      const double b = mCmnd[bestTau];
      const double c = mCmnd[bestTau + 1];
      const double denom = a - 2.0 * b + c;
      if (std::abs(denom) > 1.0e-12)
        refinedTau += 0.5 * (a - c) / denom;
    }

    const double candidate = mAnalysisRate / std::max(refinedTau, 1.0);
    const double candidateConfidence = std::clamp(1.0 - mCmnd[bestTau], 0.0, 1.0);
    if (candidate < 55.0 || candidate > 1400.0)
      return;

    if (mPitchHz <= 0.0) {
      mPitchHz = candidate;
    } else {
      const double semitoneDistance = std::abs(12.0 * std::log2(candidate / mPitchHz));
      if (semitoneDistance < 2.5)
        mPitchHz = 0.72 * mPitchHz + 0.28 * candidate;
      else if (candidateConfidence > 0.82)
        mPitchHz = candidate;
    }
    mConfidence = candidateConfidence;
  }

  std::array<double, kBufferSize> mBuffer{};
  std::array<double, kMaxTauStorage> mDifference{};
  std::array<double, kMaxTauStorage> mCmnd{};
  int mWrite = 0;
  int mFilled = 0;
  int mDecimation = 4;
  int mDecimCount = 0;
  int mSamplesUntilAnalysis = 0;
  double mDecimSum = 0.0;
  double mAnalysisRate = 12000.0;
  double mPitchHz = 0.0;
  double mConfidence = 0.0;
  double mRms = 0.0;
};

} // namespace screamback
