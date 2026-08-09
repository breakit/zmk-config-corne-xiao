#!/usr/bin/env bash
# Regenerate the keymap-drawer SVG layout images for the Corne Xiao v1.
#
# Uses the keymap-drawer CLI (pip install keymap-drawer) to reproduce the
# reference style from Townk/zmk-config: dark 42-key Corne, MDI glyphs,
# home-row-mod 'held' keys, layer keys, ghost layer-toggles, and combos.
#
# Usage:  ./media/generate.sh
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
CONFIG="${ROOT}/media/keymap-config.yaml"
KEYMAP="${ROOT}/boards/shields/corne_xiao_v1/corne_xiao_v1.keymap"
OUTDIR="${ROOT}/media"

TMP="$(mktemp).yaml"
trap 'rm -f "$TMP"' EXIT

# Parse the (LAYER_ADAPTER + keypos_def) keymap into a keymap-drawer YAML.
keymap -c "$CONFIG" parse -z "$KEYMAP" -o "$TMP"

# Layout: corne 42-key physical layout, matching the reference geometry.
draw_layer() {
  local LAYER="$1"; local OUT="$2"
  keymap -c "$CONFIG" draw -z corne -s "$LAYER" -o "${OUTDIR}/${OUT}" "$TMP"
}

draw_layer QWERTY     layer0-main.svg
draw_layer Navigation layer1-navigation.svg
draw_layer Numbers    layer2-numbers.svg
draw_layer Symbols    layer3-symbols.svg
draw_layer Media      layer4-media.svg
draw_layer Functions  layer5-functions.svg
draw_layer Buttons    layer6-buttons.svg
draw_layer System     layer7-system.svg

echo "Regenerated layout images in ${OUTDIR}"
