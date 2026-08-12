#!/usr/bin/env bash
# Regenerate the German (QWERTZ) keymap-drawer SVG layout images for the
# Corne Xiao v1.
#
# Renders the same keymap as generate.sh but with German QWERTZ symbol
# labels, from media/keymap-config-de.yaml.
#
# Usage:  ./media/generate-de.sh
# Requires: keymap-drawer CLI on PATH (pip install keymap-drawer) + pyyaml,
#           and the DE config generated (./media/generate_de_config.py).
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
CONFIG="${ROOT}/media/keymap-config-de.yaml"
KEYMAP="${ROOT}/boards/shields/corne_xiao_v1/corne_xiao_v1.keymap"
OUTDIR="${ROOT}/media"

# Regenerate the German config from the base config if missing.
if [ ! -f "$CONFIG" ]; then
  python3 "${ROOT}/media/generate_de_config.py"
fi

TMP="$(mktemp).yaml"
trap 'rm -f "$TMP"' EXIT

keymap -c "$CONFIG" parse -z "$KEYMAP" -o "$TMP"

draw_layer() {
  local LAYER="$1"; local OUT="$2"
  keymap -c "$CONFIG" draw -z corne -s "$LAYER" -o "${OUTDIR}/${OUT}" "$TMP"
}

draw_layer QWERTY     layer0-main-de.svg
draw_layer Navigation layer1-navigation-de.svg
draw_layer Numbers    layer2-numbers-de.svg
draw_layer Symbols    layer3-symbols-de.svg
draw_layer Media      layer4-media-de.svg
draw_layer Functions  layer5-functions-de.svg
draw_layer Buttons    layer6-buttons-de.svg
draw_layer System     layer7-system-de.svg

echo "Regenerated German layout images in ${OUTDIR}"
