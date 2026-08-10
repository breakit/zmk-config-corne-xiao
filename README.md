# zmk-config-corne-xiao

A [ZMK](https://zmk.dev/) firmware configuration for a **Corne Xiao v1** split keyboard, featuring a shared multi-layer layout adapted from [Townk's ZMK configuration](https://github.com/Townk/zmk-config).

The config builds split left/right firmware for the Corne Xiao (`xiao_ble//zmk`), with an OLED on the central (left) half showing **both** halves' battery, home-row modifiers, a Caps-Word combo, and Windows-oriented navigation/button layers.

> Layout guide images (`.svg`) for every layer live in [`media/`](media/) and are generated with [Keymap-Drawer](https://keymap-drawer.streamlit.app/) — run `./media/generate.sh` to regenerate them.

---

## Hardware configuration

- **Board:** Seeed Studio XIAO BLE (`xiao_ble//zmk`).
- **Shield:** `corne_xiao_v1_left` / `corne_xiao_v1_right`, plus the `rgbled_adapter` shield (wiring for optional WS2812 underglow).
- **Controller**: one XIAO per half; the **left** half is the split *central* role.
- **Display:** a 128×32 SSD1306 OLED on the central (left) half, showing a custom status screen: both halves' battery (`L<x%> R<y%>`), a WPM readout with a Bongo-Cat-style cat that plays while typing, and the active layer name.
- **Right half display:** the OLED stays in use until/unless a Cirque GlidePoint trackpad is installed in its place (see [Trackpad (optional)](#trackpad-optional)).

### Kconfig notes (`config/corne_xiao_v1.conf`)

| Setting | Purpose |
|---------|---------|
| `CONFIG_NFCT_PINS_AS_GPIOS=y` | Use the XIAO's NFC pads as GPIOs. |
| `CONFIG_ZMK_SLEEP=y` | Deep sleep for battery life. |
| `CONFIG_ZMK_DISPLAY=y` | Enables the display subsystem (LVGL + SSD1306). |
| `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y` | Central fetches the right half's battery level over BLE. |
| `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_PROXY=y` | Exposes the peripheral battery through a Battery Service. |
| `CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y` | Uses the custom OLED status screen (battery, WPM + Bongo-Cat, layer). |
| `CONFIG_ZMK_WPM=y` | Words-per-minute tracking for the status screen. |
| `CONFIG_LV_USE_LABEL=y` | LVGL label support for the custom screen. |
| `CONFIG_ZMK_HID_CONSUMER_REPORT_USAGES_FULL=y` | Full consumer HID usages (media/app shortcuts). |
| `CONFIG_ZMK_POINTING=y` | Pointing/mouse stack for the optional trackpad. |

### Trackpad (optional)

A **Cirque GlidePoint (Pinnacle) trackpad** (35 mm `TM035035` or 40 mm `TM040040`) may be wired on the right half in place of the OLED. Support is scaffolded but **off by default** so the current right-half OLED keeps working:

- Enable via `CONFIG_CORNE_XIAO_RIGHT_TRACKPAD=y` once the trackpad is installed (see the `CORNE_XIAO_RIGHT_TRACKPAD` Kconfig in `boards/shields/corne_xiao_v1/Kconfig.shield`).
- The driver comes from the [`juniorz/zmk-keyboard-cirque-pinnacle`](https://github.com/juniorz/zmk-keyboard-cirque-pinnacle) ZMK module (pulled via `config/west.yml`).

---

## Layers

There are **8 layers**. Placeholders/layer activation keys are marked in the SVG images (blue = layer, red = system).

| Image | Layer | Index |
|-------|-------|-------|
| [`layer0-main.svg`](media/layer0-main.svg) | QWERTY | 0 |
| [`layer1-navigation.svg`](media/layer1-navigation.svg) | Navigation | 1 |
| [`layer2-numbers.svg`](media/layer2-numbers.svg) | Numbers | 2 |
| [`layer3-symbols.svg`](media/layer3-symbols.svg) | Symbols | 3 |
| [`layer4-media.svg`](media/layer4-media.svg) | Media | 4 |
| [`layer5-functions.svg`](media/layer5-functions.svg) | Functions | 5 |
| [`layer6-buttons.svg`](media/layer6-buttons.svg) | Buttons | 6 |
| [`layer7-system.svg`](media/layer7-system.svg) | System | 7 |

### Layer 0 — QWERTY (base)

![QWERTY layer](media/layer0-main.svg)

The default layer.

- **Top row:** `Tab  Q  W  E  R  T` / `Y  U  I  O  P  -`
- **Home row:** `` ` `` `A⌃ S⌥ D◆ F⇧ G` / `H J⇧ K◆ L⌥ ;⌃ '` — the home-row keys double as modifiers when **held** (home-row mods). `A`=Ctrl, `S`=Alt, `D`=Win, `F`=Shift (left); `J`=Shift, `K`=Win, `L`=Alt, `;`=Ctrl (right).
- **Bottom row:** `\ Z X C V B` / `N M , . / =`
- **Left thumbs:** `System` (tap/toggle) · `Backspace` (hold = Navigation) · `Esc/Symbols` (hold)
- **Right thumbs:** `Return/Media` (hold) · `Space/Numbers` (hold) · `System` (tap/toggle)
- **Caps-Word:** press both home-row Shift keys (`F` + `J`) together to toggle Caps-Word.
- **Autoshift `V`:** tapping `V` types `v`; holding types `V`.
- Esc is available by holding the Symbols thumb key.

### Layer 1 — Navigation (Windows)

![Navigation layer](media/layer1-navigation.svg)

Cursor and window-management shortcuts.

- Arrow keys live on the right-hand home cluster; `Home`/`End`/`PageUp`/`PageDown` nearby.
- **Word/line navigation:** `Ctrl+←` (previous word), `Ctrl+→` (next word), `Home`/`End` (line start/end).
- **App switching:** `Alt+Tab` (next), `Alt+Shift+Tab` (previous) on the left.
- **Window switching:** `Alt+Esc` / `Alt+Shift+Esc`.
- **Find:** `Ctrl+G` (next), `Ctrl+Shift+G` (previous).
- **Edit shortcuts:** `Ctrl+Z/X/C/V`, `Ctrl+A/S/D/F`, `Ctrl+Q/W/E/R/T`.
- Left thumbs hold this layer (`Navigation` toggle on both sides to exit).

### Layer 2 — Numbers

![Numbers layer](media/layer2-numbers.svg)

A number block laid out over two rows (5 keys each) instead of a classic numpad:

- **Rows:** `1 2 3 4 5` on the top, `6 7 8 9 0` beneath (with `Tab`, `Enter`, and `=` anchors).
- Symbols `[ * / ] ^`, `( + - ) .` arranged on the left halves.
- Right home-row keys become modifiers when held.
- **Backspace/Delete** on a thumb (`Shift` turns it into forward Delete).
- `Space` and a `Numbers` toggle on the right thumbs.

### Layer 3 — Symbols

![Symbols layer](media/layer3-symbols.svg)

Single-sided symbols layer (mostly on the right half):

- **Right side:** `? + - . / \` / `& < = > | #` / `[ ( : ) ] %` / `$ { * } ^ !`
- **Tilde** `~` on the left outer thumb.
- **`@`**, `Space`, and a `Symbols` toggle on the right thumbs (**ghost** toggles).
- Left home-row keys act as modifiers when held.

### Layer 4 — Media

![Media layer](media/layer4-media.svg)

Media and display controls:

- **Volume:** `Vol+` / `Vol-` / `Mute`.
- **Brightness:** display brightness `+` / `-`.
- **Player:** `Previous` / `Play-Pause` / `Next` / `Stop`.
- `Media` toggle on the thumbs.

### Layer 5 — Functions

![Functions layer](media/layer5-functions.svg)

Functions keys in two 5-row blocks:

- **Top:** `F11`–`F15`; **above/below** `F6`–`F10` and `F1`–`F5`; `F16`–`F20` on the far column.
- `Functions` toggle on the left thumb.

### Layer 6 — Buttons (Windows)

![Buttons layer](media/layer6-buttons.svg)

OS/app window and desktop shortcuts plus clipboard:

- **Desktop switching:** `Ctrl+Win+Left` / `Ctrl+Win+Right` (previous/next virtual desktop).
- **Window switching:** `Ctrl+F6` / `Ctrl+Shift+F6` (next/previous window in app), `Ctrl+Tab` (in-app), `Win+Tab` (Task View).
- **Clipboard/editing:** `Ctrl+Z` (undo), `Ctrl+Y` (redo), `Ctrl+X` (cut), `Ctrl+C` (copy), `Ctrl+V` (paste).
- **Launchers:** `Win+E` (File Explorer), `Win+S` (Search), `Win+D` (Show Desktop).
- **Find:** `Ctrl+G` / `Ctrl+Shift+G`.
- `Buttons` toggle on the thumbs.

### Layer 7 — System

![System layer](media/layer7-system.svg)

Keyboard-level controls:

- **Bootloader:** `⇑` on the top-left and top-right of each half (enter DFU per half).
- **Reset:** `↺` (soft reset) on each half.
- **Power / output:** power toggle, `USB/BLE` output switching.
- **Bluetooth:** profiles `1`–`5` and `Clear` on the right half.
- `System` toggle on the thumbs.

---

## Building

This is a ZMK user config, built with the [ZMK GitHub Actions workflow](.github/workflows/build.yml). Firmware for all four targets (`corne_xiao_v1_left/right`, `corne_xiao_v2_left/right`) is produced automatically by CI on every push.

For a local build, follow [ZMK's setup instructions](https://zmk.dev/docs/development/setup/native) and build from this repo's `config/`:

```sh
west build -p -d build/v1-left  -b xiao_ble//zmk -- -DSHIELD="corne_xiao_v1_left rgbled_adapter"
west build -p -d build/v1-right -b xiao_ble//zmk -- -DSHIELD="corne_xiao_v1_right rgbled_adapter"
```

### Regenerating the layout images

The `media/*.svg` images are produced by [Keymap-Drawer](https://keymap-drawer.streamlit.app/):

```sh
pip install keymap-drawer
./media/generate.sh
```

---

## Project layout

```
config/west.yml                            # ZMK + module manifest
config/corne_xiao_v1.conf                  # Kconfig for the v1 shield
config/corne_xiao_v2.conf                  # Kconfig for the v2 shield
boards/shields/corne_xiao_v1/
  corne_xiao_v1.keymap                     # entry point: LAYER_ADAPTER + includes
  keypos_def/corne_xiao_v1.dtsi            # 42-key positional definitions
  layout/                                  # shared layout sources
    standard_layout.dtsi                   # the 8 layers
    homerowmods.dtsi, autoshift.dtsi, ...
  widgets/status_screen.c                  # OLED status screen (battery, WPM, layer)
  corne_xiao_v1_trackpad.dtsi              # Cirque trackpad wiring (disabled)
media/                                     # keymap-drawer layer images + generator
```

---

## Inspirations

This configuration is a **port of the [Townk ZMK configuration](https://github.com/Townk/zmk-config)** (by Thiago Alves), which uses a *master layout* shared across several keyboards:

- [Townk/zmk-config — Multi-keyboard ZMK configuration](https://github.com/Townk/zmk-config)

Key ideas inherited from Townk:

- A logical 60-key master layout that each keyboard maps onto its physical layout via a `LAYER_ADAPTER` macro.
- Home-row modifiers modeled on the **"timeless homerow mods"** from [Robert U (@urob)'s ZMK configuration](https://github.com/urob/zmk-config).
- Single-sided non-base layers, layered Symbols/Numbers, Caps-Word on a both-Shift combo.
- macOS-oriented layout that this port re-targets to **Windows** shortcuts.

Additional upstream references:

- [ZMK Firmware](https://github.com/zmkfirmware/zmk) — the firmware used by this config.
- The Corne Xiao boards come from [friction07/corne-xiao](https://github.com/friction07/corne-xiao).
- The Cirque GlidePoint driver module: [juniorz/zmk-keyboard-cirque-pinnacle](https://github.com/juniorz/zmk-keyboard-cirque-pinnacle).
- Keymap images rendered with [Keymap-Drawer](https://keymap-drawer.streamlit.app/) from Cem Aksoylar ([@caksoylar](https://github.com/caksoylar)).

---

### License / disclaimers

Layout concept and helper source are adapted from [Townk/zmk-config](https://github.com/Townk/zmk-config) (MIT). Firmware is built with [ZMK](https://zmk.dev/). The layout images and this documentation reflect the **Corne Xiao v1** port specifically.
