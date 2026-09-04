# SCREAMBACK

**Controlled amplifier-style feedback without an amplifier.**

SCREAMBACK is an open-source guitar feedback effect for Windows and macOS. It listens to the clean input, tracks the fundamental, and grows a controlled harmonic feedback voice that can be played from the UI, automated by the DAW, or triggered from MIDI.

## v0.1 MVP

Three feedback personalities:

- **DF-2** — old-school tracked synthetic feedback with a little vibrato and a conservative harmonic transition.
- **FreqOut** — faster, tighter harmonic reinforcement with more resonator in the sound.
- **Natural** — slower, less predictable harmonic migration: fundamental → 2nd → 3rd → 5th.

Controls are intentionally minimal:

- **Mode** — DF-2 / FreqOut / Natural
- **Voice** — Auto / Fundamental / 2nd / 3rd / 5th
- **Amount** — feedback level
- **Onset** — how quickly feedback blooms
- **Sensitivity** — tracking/gate sensitivity
- **Engage** — arm the feedbacker

Put SCREAMBACK **before distortion / amp simulation** for the cleanest tracking.

## MIDI

SCREAMBACK accepts MIDI input in VST3/AU hosts.

| Control | Mapping |
| --- | --- |
| Momentary feedback gate | Note C3 (MIDI note 60) or CC64 |
| Engage | CC20 |
| Amount | CC21 |
| Onset | CC22 |
| Voice | CC23 |
| Sensitivity | CC24 |
| Mode | CC25 |

All user-facing controls are normal plug-in parameters too, so they can be automated by the DAW.

## Formats

CI currently builds:

- Windows x64: **VST3** + standalone test app
- macOS universal (Apple Silicon + Intel): **VST3**, **AUv2** + standalone test app

The macOS CI artifacts are development builds and are not notarized yet. Proper Developer ID signing/notarization can be added to release jobs once signing credentials are configured.

## Building

SCREAMBACK uses [iPlug2](https://github.com/iPlug2/iPlug2), pinned in CI to commit `d54f69050f517e43b941d88c2a170f0a840b9ee4`.

Clone iPlug2 next to this repository and download the public plug-in SDKs:

```bash
git clone https://github.com/iPlug2/iPlug2.git ../iPlug2
git -C ../iPlug2 checkout d54f69050f517e43b941d88c2a170f0a840b9ee4
cd ../iPlug2/Dependencies/IPlug
./download-iplug-sdks.sh
cd -
```

Then configure and build with CMake. Example with Ninja:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DIPLUG2_DIR="$(pwd)/../iPlug2" \
  -DIPLUG_DEPLOY_PLUGINS=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

## DSP overview

The current engine is intentionally small and hackable:

`clean input → decimation → YIN pitch detector → harmonic selector → oscillator/resonator → onset envelope → soft limiting → dry mix`

Pitch analysis runs on a downsampled stream to keep CPU usage reasonable. The effect holds the last reliable pitch briefly while the source note decays, which lets the synthetic feedback outlive the guitar transient instead of collapsing as soon as the input gets quiet.

The DSP lives in header-only classes under `DSP/` and has a deterministic smoke test independent of the plug-in wrapper.

## Roadmap

Likely next steps after real guitar testing:

- better attack/new-note discrimination
- dual-resonator crossfades during harmonic migration
- optional Low/High natural-feedback personalities
- MIDI learn instead of fixed CC assignments
- preset browser and A/B
- signed/notarized macOS release artifacts and installers
- automated plug-in validation (`pluginval`, `auval`)

## License

SCREAMBACK is licensed under **GPL-3.0-only** with a reasonable author-attribution requirement under GPLv3 section 7(b). See `LICENSE` and `ADDITIONAL_TERMS.md`.

Required attribution when conveying the software or modified builds:

> SCREAMBACK — original concept and authorship by Ilya Tolstoukhov (@myldy20).
