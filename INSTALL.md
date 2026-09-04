# Installing SCREAMBACK

SCREAMBACK ships as a normal installer on both supported desktop platforms.

## Windows 10/11 x64

1. Download `SCREAMBACK-<version>-Windows-x64-Setup.exe` from the GitHub Release.
2. Run the installer.
3. Restart or rescan plug-ins in your DAW.

The installer places the VST3 bundle at:

`C:\Program Files\Common Files\VST3\SCREAMBACK.vst3`

It also creates a normal Windows uninstaller entry.

Current public CI builds are not Authenticode-signed, so Windows SmartScreen may show an "Unknown publisher" warning. The binary is built directly by GitHub Actions from this public repository.

## macOS (Apple Silicon + Intel)

1. Download `SCREAMBACK-<version>-macOS-universal.pkg` from the GitHub Release.
2. Open the package and complete the installer.
3. Restart or rescan plug-ins in your DAW.

The package installs:

- VST3: `/Library/Audio/Plug-Ins/VST3/SCREAMBACK.vst3`
- AUv2: `/Library/Audio/Plug-Ins/Components/SCREAMBACK.component`

The plug-in bundles are ad-hoc signed in CI. The installer package itself is not Developer ID signed or notarized yet, so macOS may block the first launch. If that happens, open **System Settings → Privacy & Security** and choose **Open Anyway** for the installer, then run it again.

## Manual installation

GitHub Actions artifacts also contain the raw plug-in bundles for development/testing.

### Windows

Copy `SCREAMBACK.vst3` to:

`C:\Program Files\Common Files\VST3\`

### macOS

Copy:

- `SCREAMBACK.vst3` to `/Library/Audio/Plug-Ins/VST3/`
- `SCREAMBACK.component` to `/Library/Audio/Plug-Ins/Components/`

User-local installation under `~/Library/Audio/Plug-Ins/` also works on macOS.

## Recommended signal chain

For the cleanest pitch tracking, place SCREAMBACK before distortion, amp simulation, cabinet IRs and large time-based effects.
