# Build Spikeout for iPhone (from a Mac)

This guide is for putting **Spikeout** on an iPhone yourself from **macOS**.
The phone build uses the **interpreter** (iOS does not allow a JIT) and draws with
**Metal**. You need your own `spikeofe` ROM set; **never commit ROM files**.

| | |
| --- | --- |
| Mac | macOS with **Xcode 15+** (App Store) |
| Tools | **CMake** and **Ninja** (e.g. `python3 -m pip install --user cmake ninja`) |
| Phone | **iOS 16.3+**, unlocked, Developer Mode on, trusted to this Mac |
| Apple ID | Free **Personal Team** is enough for short sideload; paid team for longer installs |
| ROM | Your own `spikeofe` folder or `spikeofe.zip` (same as MAME / Supermodel) |

Find your **Team ID** in Xcode → Settings → Accounts → your Apple ID → Team,
or at [developer.apple.com/account](https://developer.apple.com/account) → Membership.

---

## 1. Get the source (no ROMs)

```sh
git clone https://github.com/hkpcqwe/spikeout-recomp.git
cd spikeout-recomp
```

Put CMake on your `PATH` if needed:

```sh
export PATH="$HOME/Library/Python/3.9/bin:$HOME/.local/bin:/opt/homebrew/bin:$PATH"
```

---

## 2. Build for a real iPhone

Plug in the phone, unlock it, tap **Trust** if asked. Then:

```sh
./tools/ios-build.sh --device \
  --rom /path/to/spikeofe \
  --team YOUR_TEAM_ID
```

What that does:

1. Builds shaders once on the Mac (`build-ios-host/shaders`)
2. Makes an Xcode iOS project and a **Release** app (`Spikeout`)

The signed app is usually:

`build-ios/Release-iphoneos/Spikeout.app`

Install and launch from the Mac:

```sh
APP=build-ios/Release-iphoneos/Spikeout.app

# If spikeofe is missing inside the .app (rare), copy it then re-sign in Xcode, or rebuild with --rom.
xcrun devicectl device install app --device "Your iPhone Name" "$APP"
xcrun devicectl device process launch --device "Your iPhone Name" \
  --terminate-existing --activate com.spikeout.recomp
```

Or open the project and press Run:

```sh
./tools/ios-build.sh --device --rom /path/to/spikeofe --team YOUR_TEAM_ID --open
# Xcode → scheme spikeout-interp → your iPhone → ▶
```

First launch on a free Personal Team: Settings → General → VPN & Device Management →
trust the developer, then open **Spikeout** again.

---

## 3. Sideload (no Xcode Run button)

You can install the same **Release** build with a sideloader tool. Build the `.app`
first (section 2), then make an `.ipa` and feed it to the tool.

### Make an `.ipa` from `Spikeout.app`

```sh
APP=build-ios/Release-iphoneos/Spikeout.app
OUT="$PWD/build-ios/Spikeout.ipa"
rm -rf /tmp/spikeout-ipa && mkdir -p /tmp/spikeout-ipa/Payload
cp -R "$APP" /tmp/spikeout-ipa/Payload/
( cd /tmp/spikeout-ipa && zip -qry "$OUT" Payload )
echo "IPA: $OUT"
```

Do **not** upload this IPA (or any build that contains `spikeofe`) to the internet.
Keep it for your own phone.

### Sideloadly (Mac or Windows)

[Sideloadly](https://sideloadly.io) is the usual choice for a self-built IPA.

1. Install Sideloadly on the Mac (or a Windows PC with Apple devices support).
2. Plug in the iPhone, unlock it, trust the computer.
3. Open Sideloadly → select your iPhone → drag in `build-ios/Spikeout.ipa`.
4. Sign in with **your** Apple ID (the one you use for signing).
5. Start the install. When it finishes, open **Spikeout** on the phone.

Free Apple ID: the app expires after **7 days**; run Sideloadly again to refresh.
A paid Apple Developer account lasts longer (typically a year per signing).

First open after sideload: **Settings → General → VPN & Device Management** →
trust your Apple ID / developer profile.

### AltStore (Mac)

[AltStore](https://altstore.io) / AltServer is another option if you already use it.

1. Install AltServer on the Mac and AltStore on the iPhone (follow their setup).
2. In AltStore → **My Apps** → **+** → pick `Spikeout.ipa`.
3. Sign with your Apple ID when asked.

Same 7-day free-account limit unless you use a paid team. Wi‑Fi refresh needs
AltServer running on the Mac on the same network (or use a USB refresh).

### `devicectl` (Mac, already signed by Xcode)

If the `.app` was signed with your team during `ios-build.sh --device`, you can
install without a GUI sideloader:

```sh
xcrun devicectl device install app --device "Your iPhone Name" \
  build-ios/Release-iphoneos/Spikeout.app
```

That is still “sideloading” in practice; Sideloadly/AltStore are better when you
want to re-sign with another Apple ID or install from a Windows machine.

---

## 4. Simulator (optional)

No signing needed:

```sh
./tools/ios-build.sh --simulator --rom /path/to/spikeofe

APP=build-ios/Release-iphonesimulator/Spikeout.app
xcrun simctl boot "iPhone 16" 2>/dev/null || true
open -a Simulator
xcrun simctl install booted "$APP"
xcrun simctl launch booted com.spikeout.recomp
```

The Simulator is useful for UI checks; speed is not the same as a real phone.

---

## 5. ROM without rebuilding

The app allows Files / Finder sharing. Put `spikeofe.zip` or a `spikeofe` folder
into the app’s Documents, then relaunch. Or pass `--rom` at build time to bundle
it for your own sideload only (**do not publish apps that include ROM data**).

---

## 6. On the phone

- **Touch:** left stick, COIN / START, AT / CH / JP / SH, and **MENU** (opens the
  in-game menu; tap menu rows to choose; MENU again or Resume to close)
- **Pads:** Bluetooth / MFi / DualSense / Xbox work alongside touch
- The iOS build pins a light picture (scale 1, no AA) so older phones stay playable;
  FPS can be shown in the corner

iPhone draws at Retina size. An “Auto” render scale that follows the pixel size
is too heavy with the interpreter. The iOS build forces a cheap default; raise
quality later in the menu only if the phone keeps up.

---

## 7. Script options

```text
./tools/ios-build.sh --device|--simulator
                     --rom /path/to/spikeofe|/path/to/spikeofe.zip
                     --team TEAMID
                     --open          # open the Xcode project when done
```

Environment: `SPIKEOUT_IOS_TEAM` = default Team ID.

---

## Notes

- Internet lobby is off in the iOS build for now (LAN / offline only)
- Never commit ROM dumps or anything extracted from them
- App Store distribution needs Apple review and must not ship ROM files
