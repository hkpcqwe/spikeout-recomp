#!/usr/bin/env bash
# Build Spikeout for iOS (interpreter + Metal). Two stages:
#   1) host macOS build → compile GLSL shaders once
#   2) iOS Xcode project → spikeout-interp.app (Simulator or device)
#
# Usage:
#   tools/ios-build.sh                  # Simulator (arm64)
#   tools/ios-build.sh --device         # connected iPhone (needs signing team)
#   tools/ios-build.sh --rom DIR        # bundle your spikeofe folder/zip into the app
#   tools/ios-build.sh --team TEAMID    # Apple Developer Team ID
#   tools/ios-build.sh --open           # open the Xcode project when done
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
export PATH="${HOME}/Library/Python/3.9/bin:${HOME}/.local/bin:/opt/homebrew/bin:/usr/local/bin:${PATH}"

DEST=simulator
ROM_DIR=""
TEAM="${SPIKEOUT_IOS_TEAM:-}"
OPEN=0
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --device) DEST=device; shift ;;
    --simulator) DEST=simulator; shift ;;
    --rom) ROM_DIR="$2"; shift 2 ;;
    --team) TEAM="$2"; shift 2 ;;
    --open) OPEN=1; shift ;;
    -h|--help)
      sed -n '2,12p' "$0"
      exit 0
      ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

if ! command -v cmake >/dev/null; then
  echo "cmake not found; install with: python3 -m pip install --user cmake ninja" >&2
  exit 1
fi

HOST="${ROOT}/build-ios-host"
IOS="${ROOT}/build-ios"

echo "==> [1/2] Host shaders (macOS)"
cmake -S "${ROOT}" -B "${HOST}" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DSPIKEOUT_BUILD_TESTS=OFF \
  -DSPIKEOUT_LOBBY=OFF \
  -DSPIKEOUT_LLVM_JIT=OFF \
  -DSPIKEOUT_INTERP=ON \
  -DSPIKEOUT_FRONTEND=ON \
  -DSPIKEOUT_GPU=ON \
  -DSPIKEOUT_TOOLS=ON
cmake --build "${HOST}" --target m3gpu -j "${JOBS}"

if [[ ! -d "${HOST}/shaders" ]]; then
  echo "host shaders missing at ${HOST}/shaders" >&2
  exit 1
fi

SDK=iphonesimulator
ARCH=arm64
if [[ "${DEST}" == device ]]; then
  SDK=iphoneos
fi

CMAKE_EXTRA=()
if [[ -n "${ROM_DIR}" ]]; then
  CMAKE_EXTRA+=(-DSPIKEOUT_IOS_ROM_DIR="${ROM_DIR}")
fi
if [[ -n "${TEAM}" ]]; then
  CMAKE_EXTRA+=(-DSPIKEOUT_IOS_TEAM="${TEAM}")
fi

echo "==> [2/2] iOS (${SDK}, ${ARCH})"
cmake -S "${ROOT}" -B "${IOS}" -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT="${SDK}" \
  -DCMAKE_OSX_ARCHITECTURES="${ARCH}" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=16.3 \
  -DSPIKEOUT_PREBUILT_SHADERS="${HOST}/shaders" \
  -DSPIKEOUT_BUILD_TESTS=OFF \
  -DSPIKEOUT_LOBBY=OFF \
  -DSPIKEOUT_LLVM_JIT=OFF \
  -DSPIKEOUT_INTERP=ON \
  -DSPIKEOUT_FRONTEND=ON \
  -DSPIKEOUT_GPU=ON \
  -DSPIKEOUT_TOOLS=OFF \
  "${CMAKE_EXTRA[@]}"

CONFIG=Release
if [[ "${DEST}" == simulator ]]; then
  cmake --build "${IOS}" --config "${CONFIG}" --target spikeout-interp -- \
    -sdk "${SDK}" -arch "${ARCH}" CODE_SIGNING_ALLOWED=NO
else
  cmake --build "${IOS}" --config "${CONFIG}" --target spikeout-interp -- \
    -sdk "${SDK}" -arch "${ARCH}" \
    -allowProvisioningUpdates -allowProvisioningDeviceRegistration
fi

APP="$(find "${IOS}" -path "*${CONFIG}*" -name 'spikeout-interp.app' -type d | head -n 1 || true)"
if [[ -z "${APP}" ]]; then
  APP="$(find "${IOS}" -name 'spikeout-interp.app' -type d | head -n 1 || true)"
fi

echo
echo "Built: ${APP:-"(open ${IOS}/spikeout-recomp.xcodeproj)"}"
echo "Xcode project: ${IOS}/spikeout-recomp.xcodeproj"

if [[ "${DEST}" == simulator && -n "${APP}" ]]; then
  echo
  echo "Install on Simulator:"
  echo "  xcrun simctl boot 'iPhone 16' 2>/dev/null || true"
  echo "  xcrun simctl install booted \"${APP}\""
  echo "  xcrun simctl launch booted com.spikeout.recomp"
fi

if [[ "${OPEN}" -eq 1 ]]; then
  open "${IOS}/spikeout-recomp.xcodeproj"
fi
