<p align="center"><img src="assets/icon/icon.png" alt="Spikeout on iPhone" width="160"></p>

<h1 align="center">Spikeout on iPhone</h1>

<p align="center"><b>SEGA’s arcade Spikeout: Final Edition on your iPhone — including older devices.
Built for sideload from a Mac: on-screen touch, Metal, and graphics tuned so an iPhone 11–class phone stays smooth.</b></p>

<p align="center">
  <img alt="iOS" src="https://img.shields.io/badge/iOS-16.3%2B%20sideload-blue">
  <img alt="Mac build" src="https://img.shields.io/badge/build-macOS%20%2B%20Xcode-lightgrey">
  <a href="LICENSE"><img alt="Licence: GPL-3.0" src="https://img.shields.io/badge/licence-GPL--3.0-blue"></a>
  <a href="https://buymeacoffee.com/hkpcqwe"><img alt="Buy me a coffee" src="https://img.shields.io/badge/Buy%20me%20a%20coffee-FFDD00?logo=buymeacoffee&logoColor=black"></a>
</p>

<p align="center">
  <a href="docs/building-ios.md">Install on iPhone</a> ·
  <a href="#why-this-fork">Why this fork</a> ·
  <a href="#play-on-iphone">Play</a> ·
  <a href="#controls">Controls</a> ·
  <a href="#support">Buy me a coffee</a> ·
  <a href="#faq">FAQ</a>
</p>

> [!IMPORTANT]
> **No game files are included** — no ROMs and nothing made from them. You need your own `spikeofe` set
> (`spikeofe.zip` / folder), the same one MAME and Supermodel use. Unofficial fan project: not made, approved,
> or supported by SEGA.

## Why this fork

This tree is aimed at **playing Spikeout on iOS**.

| | Desktop recomp (upstream) | **This fork (iOS)** |
| --- | --- | --- |
| Focus | Windows / Linux / macOS / Steam Deck | **iPhone sideload** |
| How the game runs | LLVM JIT (or AOT build) | **Interpreter** (iOS forbids JIT) |
| Drawing | Vulkan / Metal on PC | **Metal**, cheap defaults for phones |
| Input | Keyboard / pad | **On-screen touch** + Bluetooth pads |
| Install | Download a zip | **Build on a Mac**, then Xcode / Sideloadly / AltStore |

On a real **iPhone 11 / 11 Pro Max**, the build pins a light picture (render scale 1, no AA, no heavy Retina
overdraw) so attract and play stay usable. Open the in-game **MENU**, tap rows with your finger, and close with
MENU again or Resume.

Upstream PC project (full desktop downloads, netplay, roadmap):
[tastyrepublic/spikeout-recomp](https://github.com/tastyrepublic/spikeout-recomp).  
This repo keeps that engine under **GPL-3.0** and adds the iOS path, touch UI, and phone-oriented defaults.

## Play on iPhone

**You need:** a Mac with Xcode 15+, your Apple ID (free Personal Team is enough for short sideload), an iPhone on
**iOS 16.3+**, and your own `spikeofe` ROM.

Full guide (Xcode, **Sideloadly**, **AltStore**, `devicectl`, making an `.ipa`):

### → [docs/building-ios.md](docs/building-ios.md)

Short version:

```sh
git clone https://github.com/hkpcqwe/spikeout-recomp.git
cd spikeout-recomp
export PATH="$HOME/Library/Python/3.9/bin:$HOME/.local/bin:/opt/homebrew/bin:$PATH"

./tools/ios-build.sh --device --rom /path/to/spikeofe --team YOUR_TEAM_ID
```

Then either:

- Run from Xcode, or  
- Zip `Spikeout.app` into an `.ipa` and install with **Sideloadly** / **AltStore** (steps in the guide)

Never publish an IPA that still contains ROM data.

## Controls

| On screen | Action |
| --- | --- |
| Left stick | Move |
| COIN / START | Coin, start |
| AT / CH / JP / SH | Beat, charge, jump, shift |
| **MENU** | Open / close the Esc menu (tap list rows to choose) |

Bluetooth / MFi / DualSense / Xbox pads work alongside touch.

## Status (iOS)

| Area | State |
| --- | --- |
| Attract, play, sound, music | Working on device (interpreter + Metal) |
| Touch + MENU | Working |
| Older phones (e.g. iPhone 11) | Tuned for smooth play (low render cost by default) |
| Internet lobby | Off for now (LAN / offline only on iOS) |
| App Store | Not submitted — sideload only |

## Desktop / other platforms

Windows, Linux, Steam Deck and macOS builds and netplay are maintained upstream. You can still build them from
this tree; for ready-made PC downloads and the full desktop README, use
[tastyrepublic/spikeout-recomp](https://github.com/tastyrepublic/spikeout-recomp).

Mac desktop build notes: [docs/building-macos.md](docs/building-macos.md).

## FAQ

**Where do I get the ROM?** From your own board / dump. This project can’t help you find one.

**Do I need a paid Apple Developer account?** No for trying it. Free Apple ID sideloads last about **7 days**, then
re-install with Sideloadly / Xcode. A paid team lasts longer.

**Is this the same as the PC download?** Same game engine family; on iPhone it uses the interpreter and phone UI.
PC zips are from the upstream project.

**Can I put this on the App Store?** Not with ROM files inside. Any store build must not ship SEGA’s game data.

## Support

If this iOS build helped you play on the go (or on an older phone), a coffee helps cover devices, Apple fees, and
time.

<p align="center">
  <a href="https://buymeacoffee.com/hkpcqwe"><img alt="Buy me a coffee" src="https://img.shields.io/badge/Buy%20me%20a%20coffee-hkpcqwe-FFDD00?style=for-the-badge&logo=buymeacoffee&logoColor=black"></a>
</p>

**Set up your own Buy Me a Coffee page (one-time):**

1. Open [buymeacoffee.com/signup](https://buymeacoffee.com/signup) and pick a username (this README uses `hkpcqwe` —
   change the links if you choose another).
2. Confirm email, add a short bio (e.g. “Spikeout on iPhone — sideload builds”).
3. Connect **payouts** (Stripe) — the page stays hidden until payout is set.
4. Share `https://buymeacoffee.com/YOUR_USERNAME`. GitHub also shows a **Sponsor** / funding link from
   [`.github/FUNDING.yml`](.github/FUNDING.yml).

Also helps: star the repo, report bugs, and tell others to bring their own ROM.

## Licence and credit

GPL-3.0-only — see [LICENSE](LICENSE).

Based on [tastyrepublic/spikeout-recomp](https://github.com/tastyrepublic/spikeout-recomp) (Don / tastyrepublic, with
AI-assisted development). Hardware behaviour follows [Supermodel](https://github.com/trzy/Supermodel) first, then MAME.
Third-party libraries and notices: see the upstream project and `THIRD-PARTY-NOTICES` in desktop releases.

Not affiliated with SEGA.
