#!/usr/bin/env bash
# Build chastity firmware and copy UF2s into the firmware/ distribution folder.
# Usage: ./build.sh [left|right|both]
set -euo pipefail

cd "$(dirname "$0")"
TARGET="${1:-both}"
DEST="/workspaces/chastity/firmware/choc/v1/zmk-build"

build_half() {
    local half="$1"
    rm -rf "build/${half}"
    west build -s zmk/app -d "build/${half}" -p always \
        -b "nice_nano@2.0.0/nrf52840/zmk" \
        -- "-DSHIELD=chastity_${half}" \
           "-DZMK_CONFIG=/workspaces/chastity/code/zmk/config" \
           "-DBOARD_ROOT=/workspaces/chastity/code/zmk"
    cp "build/${half}/zephyr/zmk.uf2" "${DEST}/chastity_${half}.uf2"
    echo "==> Copied chastity_${half}.uf2 to ${DEST}"
}

case "${TARGET}" in
    left)  build_half left ;;
    right) build_half right ;;
    both)  build_half left; build_half right ;;
    *) echo "Usage: $0 [left|right|both]"; exit 1 ;;
esac

echo
echo "Final state of ${DEST}:"
ls -la "${DEST}"
