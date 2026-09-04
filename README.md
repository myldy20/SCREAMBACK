# SCREAMBACK

**Controlled amplifier-style feedback without an amplifier.**

SCREAMBACK is an open-source guitar feedback effect for Windows and macOS. Put it before distortion / amp simulation, play a note, then engage **SCREAM** to grow and hold a controlled feedback voice.

## v0.2

The feedbacker now behaves like a pedal rather than a timed sound effect:

1. Play or sustain a note.
2. Press **SCREAM**. SCREAMBACK captures the detected pitch and grows feedback according to **RISE**.
3. The captured feedback keeps sounding while SCREAM is engaged, even after the guitar note itself becomes too quiet to track.
4. Release/retrigger SCREAM to capture a different note. Normal vibrato and bends around the captured note are still followed.

Three personalities:

- **DF-2** — synthetic, old-school PLL-like feedback. `Auto` stays on the fundamental.
- **FreqOut** — tighter, focused feedback. `Auto` stays on the 2nd harmonic.
- **Natural** — softer feedback that evolves slowly through harmonics when `Auto` is selected.

Controls are intentionally small in number:

- **LEVEL** — volume/intensity of the generated feedback voice.
- **RISE** — how long the feedback takes to bloom, from 15 to 1500 ms.
- **TRACK** — how easily the input detector accepts a note. Turn it up for quieter DI signals; turn it down if noisy input causes false locks.
- **HARMONIC** — Auto / Fundamental / 2nd / 3rd / 5th. Explicit harmonic choices never migrate by themselves.
- **SCREAM** — click in the UI to latch/unlatch. MIDI C3 or CC64 works as a momentary footswitch.

The default is intentionally straightforward: **FreqOut + 2nd harmonic + 250 ms rise**.

## Download and install

Tagged GitHub Releases publish one-file installers automatically:

- Windows x64: `SCREAMBACK-<version>-Windows-x64-Setup.exe`
- macOS universal (Apple Silicon + Intel): `SCREAMBACK-<version>-macOS-universal.pkg`

The Windows installer puts the VST3 bundle in the standard system VST3 folder and adds a normal uninstaller entry. The macOS package installs both VST3 and AUv2 into the standard system Audio Plug-Ins folders.

Development CI artifacts also contain the raw plug-in bundles for manual installation/testing. See [INSTALL.md](INSTALL.md) for paths and Gatekeeper/SmartScreen notes.

Current public builds do not yet use paid platform signing certificates: Windows may show SmartScreen's "Unknown publisher" warning and the macOS package is not Developer ID signed/notarized. The macOS plug-in bundles themselves are ad-hoc signed in CI.

## MIDI / automation

SCREAMBACK accepts MIDI input in VST3/AU hosts. All controls are also normal plug-in parameters and can be automated by the DAW.

| Control | Mapping |
| --- | --- |
| Momentary SCREAM | Note C3 (MIDI note 60) or CC64 |
| SCREAM toggle | CC20 |
| LEVEL | CC21 |
| RISE | CC22 |
| HARMONIC | CC23 |
| TRACK | CC24 |
| MODE | CC25 |

## Formats

CI builds and packages:

- Windows x64: **VST3** + `.exe` installer
- macOS universal (Apple Silicon + Intel): **VST3**, **AUv2** + `.pkg` installer

A tag matching `v*` automatically builds both platforms and publishes the two installers as GitHub Release assets.

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

`clean input → downsampled YIN detector → note latch → harmonic selector → oscillator/resonator → rise/release envelope → dry mix`

The pitch detector runs continuously, but once SCREAM is engaged the current note is latched. Small pitch movement is followed for bends/vibrato; large detector jumps are ignored until SCREAM is retriggered. This prevents a dying guitar note from turning into octave roulette.

The dry guitar path is left untouched; saturation/limiting is applied only to the generated feedback voice.

## Roadmap

Likely next steps after more real-guitar testing:

- better automatic new-note retriggering without sacrificing pitch stability
- dual-resonator crossfades for even more natural harmonic transitions
- optional Low/High natural-feedback personalities
- MIDI learn instead of fixed CC assignments
- preset browser and A/B
- Developer ID / Authenticode signing and macOS notarization
- automated plug-in validation (`pluginval`, `auval`)

## License

SCREAMBACK is licensed under **GPL-3.0-only** with an author-attribution requirement under GPLv3 section 7(b). See `LICENSE` and `ADDITIONAL_TERMS.md`.

Required attribution when conveying the software or modified builds:

> SCREAMBACK — original concept and authorship by Ilya Tolstoukhov (@myldy20).
