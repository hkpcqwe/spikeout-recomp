# Building for iOS

Spikeout on iPhone uses the **pre-decoded interpreter** (`spikeout-interp`): iOS
does not allow JIT, so the game is not LLVM-translated. Metal draws on a real
device; the Simulator often falls back to the software picture.

Requires: Xcode 15+, CMake 3.20+, Ninja (host shaders), your own `spikeofe` ROM
set. Deployment target **iOS 16.3+**.

## One-shot build

```sh
# PATH needs cmake (e.g. python3 -m pip install --user cmake ninja)
export PATH="$HOME/Library/Python/3.9/bin:$PATH"

# Simulator (no signing)
./tools/ios-build.sh --simulator --rom /path/to/spikeofe

# Real iPhone (Apple Developer Team ID)
./tools/ios-build.sh --device --rom /path/to/spikeofe --team YOUR_TEAM_ID --open
```

What it does:

1. Host macOS build → compiles GLSL → MSL once (`build-ios-host/shaders`)
2. Xcode iOS project → `build-ios/spikeout-recomp.xcodeproj`, target **spikeout-interp**
   (product name **Spikeout**)

## Install on Simulator

```sh
APP=build-ios/Release-iphonesimulator/Spikeout.app
# If the ROM wasn't copied by the post-build step:
#   cp -R /path/to/spikeofe "$APP/spikeofe"
xcrun simctl boot "iPhone 17" 2>/dev/null || true
open -a Simulator
xcrun simctl install booted "$APP"
xcrun simctl launch booted com.spikeout.recomp
```

## Install on a real iPhone

1. Open `build-ios/spikeout-recomp.xcodeproj` in Xcode
2. Select the **spikeout-interp** scheme, your plugged-in iPhone as destination
3. Signing & Capabilities → your Team (Personal Team works for 7-day sideload)
4. Run (▶)

Or from the CLI after setting the team:

```sh
./tools/ios-build.sh --device --rom /path/to/spikeofe --team YOUR_TEAM_ID
```

## ROM without rebuilding

`UIFileSharingEnabled` is on. In Finder (or the Files app) put `spikeofe.zip`
or a `spikeofe` folder into the app’s Documents, then relaunch.

## Controls

- **On-screen touch** (always on for iOS builds): left stick, COIN / START,
  face buttons AT (beat) / CH (charge) / JP (jump) / SH (shift), and MENU
  (opens the in-game menu when Metal/GPU is up)
- **Bluetooth / MFi / PS / Xbox** pads still work through SDL alongside touch
- On the Simulator you can also click the virtual controls with the mouse;
  or use I/O → Game Controller

## Notes

- Internet lobby is off in the iOS build (no OpenSSL); LAN / offline only for now
- Never commit ROM data; `--rom` is for personal sideload only
- App Store distribution needs Apple review and must not ship ROM files
