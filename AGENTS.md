# AGENTS.md

External QMK **userspace** for a **Sofle rev1** split keyboard running
[Miryoku](https://github.com/manna-harbour/miryoku). This repository replaced the
old fork (`~/qmk_firmware`, branch `julrich`); QMK is a pinned build input now,
not a fork to maintain.

```
qmk_firmware/                          git submodule, pinned QMK commit
keyboards/sofle/keymaps/miryoku/       Sofle keymap (this is where our code lives)
  config.h                             LAYOUT_miryoku mapping + RGB/LED config
  keymap.c                             hardware support: RGB, both OLEDs, encoders
  rules.mk                             USER_NAME, drivers, bootloader
users/manna-harbour_miryoku/           vendored Miryoku user space (layers)
qmk.json                               build targets for `qmk userspace-compile` / CI
Makefile                               forwards `make <kb>:<km>` to the submodule
```

## 1. Pins — update deliberately

| component | pin |
|---|---|
| QMK firmware | submodule `b1aea2556040c28d58d611c8aef0082b5780bb6d` (master, 2026-09-26) |
| Miryoku QMK | vendored from [yehorb/qmk_userspace@`2a827f9d`](https://github.com/yehorb/qmk_userspace/tree/miryoku) (2026-05-27), unmodified |

**Never hand-edit `users/manna-harbour_miryoku/`.** It is third-party code
(MPL/GPL, © Manna Harbour) with no upstream userspace release yet; re-vendor it
and diff. Our own Miryoku tweaks go in `users/manna-harbour_miryoku/custom_config.h`
(the mechanism Miryoku provides for exactly that) — currently empty, as we run
Miryoku defaults.

Bump QMK:

```sh
cd qmk_firmware && git fetch --depth 1 origin master && git checkout FETCH_HEAD
cd .. && git add qmk_firmware && git commit -m "chore: bump QMK to <sha>"
# then re-run both build paths below and update the pin in README.md
```

## 2. Build (all three paths verified)

The `qmk` CLI is installed with **pipx** (venv `~/.local/share/pipx/venvs/qmk`,
entry point `~/.local/bin/qmk`), with QMK's own Python deps injected plus
`appdirs`, which the frozen old fork's 2022-era CLI still imports:

```sh
pipx install qmk
pipx inject qmk -r "$(realpath qmk_firmware)/requirements.txt" appdirs
```

`avr-gcc` and QMK's AVR submodules are also required:

```sh
git submodule update --init --depth 1 qmk_firmware
cd qmk_firmware && git submodule update --init --depth 1 lib/lufa lib/printf
```

> **Trap: `QMK_HOME` defaults to `~/qmk_firmware`.** As long as the old fork
> lives there, a bare `qmk compile …` run from anywhere (including this
> repository) uses *that* tree — it looks for keymaps inside the fork and loads
> its 2022-era CLI code. Always pin the tree, or use the Makefile path, which
> points at `./qmk_firmware`.

```sh
# a) explicit env vars — no global config touched
QMK_HOME="$(realpath qmk_firmware)" QMK_USERSPACE="$(realpath .)" \
  qmk compile -kb sofle/rev1 -km miryoku

# b) project Makefile (uses the submodule; SKIP_GIT=1 avoids pulling unused submodules)
make sofle/rev1:miryoku SKIP_GIT=1
make sofle/rev1:miryoku:flash SKIP_GIT=1

# c) all qmk.json targets, hex lands in this directory
QMK_HOME="$(realpath qmk_firmware)" QMK_USERSPACE="$(realpath .)" qmk userspace-compile
```

Optional one-off, if you no longer need to build the old fork and want plain
`qmk compile -kb sofle/rev1 -km miryoku` to work from anywhere:

```sh
qmk config user.qmk_home="$(realpath qmk_firmware)" user.overlay_dir="$(realpath .)"
```

Current size: **27654 / 28672 bytes (96%, 1018 free)** on `atmega32u4`. That is
the binding constraint — check the number after every change, see §5.

## 3. What is in the keymap (and why)

`keymap.c` contains **only** Sofle-specific hardware support; the layers come
from the vendored Miryoku tree via `INTROSPECTION_KEYMAP_C` in
`users/manna-harbour_miryoku/rules.mk`.

| feature | implementation |
|---|---|
| Per-key LEDs | **RGB Matrix** with the Sofle's own LED layout (72 LEDs, 58 under keys, in `keyboards/sofle/info.json`). Default mode `RGB_MATRIX_SOLID_REACTIVE_SIMPLE` — the key you press pulses and fades. Static blue accents are drawn in `rgb_matrix_indicators_advanced_user()` from `accent_leds[]` |
| OLED (master) | `print_status_narrow()`, rotated 270°: small logo, `TSNM`, compiled alphas, active layer name |
| OLED (slave) | `render_logo()`, 128x32 bitmap, default rotation |
| Encoders | `encoder_update_user()`: index 0 = volume, index 1 = `MS_WHLU`/`MS_WHLD` |

Effects: RGB Matrix ships **47** animations. To add one, define its
`ENABLE_RGB_MATRIX_*` plus, for the keypress-driven ones, `RGB_MATRIX_KEYPRESSES`
(already set) in `config.h`; budget ~100–400 bytes each. Reactive family:
`SOLID_REACTIVE_SIMPLE`, `SOLID_REACTIVE`, `SOLID_REACTIVE_WIDE`, `..._MULTIWIDE`,
`..._CROSS`, `..._MULTICROSS`, `..._NEXUS`, `..._MULTINEXUS`, `SPLASH`,
`MULTISPLASH`, `SOLID_SPLASH`, `SOLID_MULTISPLASH`. RGBLIGHT, by contrast, has
only 10 strip-wide effects and **no** keypress reactivity — which is why the
lighting moved off it (commit `aa0d718` is the RGBLIGHT version; revert it to
go back).

Three deliberate cleanups vs. the old fork:

- The master OLED's layer names are **generated from `MIRYOKU_LAYER_LIST`**, so
  they cover all ten Miryoku layers and follow `custom_config.h` and build
  options automatically; the old hand-written `switch` only knew 8 layers.
  The alphas label (`CmkDH`) is derived from `MIRYOKU_ALPHAS_*` at compile time
  instead of being hardcoded.
- `HSV_KSDSPRI` / `HSV_KSDSSEC`, which the old fork patched into
  `quantum/color.h`, were never referenced by any code and are not carried over.
  Core files cannot be patched from a userspace — if you need custom colours,
  define them in this keymap's `config.h` (or `custom_config.h`).
- Nothing about the strip itself (pin, LED count, split) is repeated in
  `config.h`: `ws2812.pin`, `rgblight`/`rgb_matrix` counts and the LED layout all
  come from the keyboard.

## 4. Flashing

```sh
make sofle/rev1:miryoku:flash SKIP_GIT=1
# or: QMK_HOME="$(realpath qmk_firmware)" QMK_USERSPACE="$(realpath .)" qmk flash -kb sofle/rev1 -km miryoku
```

`dfu-programmer` is required and **is not currently installed** (a system
cleanup removed it, along with `avrdude`/`dfu-util`, on 2025-12-14):

```sh
sudo pacman -S dfu-programmer
```

udev rules for user-level USB access already exist
(`/etc/udev/rules.d/50-qmk.rules`, from Jan 2023). Put the half into DFU mode
(double-tap reset) and flash.

`BOOTLOADER = atmel-dfu` is set in the keymap's `rules.mk`; keymap `rules.mk` is
included after the keyboard's generated rules, so it overrides the Sofle's stock
`caterina` (verified: `-DBOOTLOADER_ATMEL_DFU` in the build flags).
`MASTER_LEFT`, no `EE_HANDS` → flash **both** halves with the same hex.

## 5. Conventions / traps

- Keymap directory names must match `[a-z0-9_]+`; the Miryoku user directory is
  linked by `USER_NAME = manna-harbour_miryoku` in the keymap `rules.mk`, not by
  name.
- The 72 WS2812 LEDs (36/side) are driven by **RGB Matrix**; `RGBLIGHT_ENABLE` is
  explicitly off. Enabling both does not fit: RGBLIGHT + RGB Matrix = 29636/28672
  (**964 bytes over**). RGB Matrix alone = 27654, RGBLIGHT alone = 26890.
  If RGBLIGHT is ever re-enabled, `RGB_MATRIX_DRIVER = WS2812` no longer exists —
  the valid value is `ws2812`.
- The old RGBLIGHT accent pattern addressed the right half as `35 + N`, which is
  off by one (the right-hand indicator LED is **36**, not 35), so the two halves
  were not lit symmetrically. `accent_leds[]` reproduces the LEDs that were
  *actually* on; fixing them to be symmetric is a one-line change if you want it.
- Modern names: `RGBLIGHT_LED_COUNT` (not `RGBLED_NUM`), `UG_*` (not `RGB_*`),
  `MS_WHLU`/`MS_WHLD` (not `KC_WH_U`/`KC_WH_D`), lower-case driver names
  (`OLED_DRIVER = ssd1306`).
- Miryoku build options keep working and are exercised in CI-less local builds:
  `make sofle/rev1:miryoku MIRYOKU_ALPHAS=QWERTY` (or `qmk compile -e ...`).
  Values are case-insensitive; see `.github/workflows/test-all-configs.yml` in
  the Miryoku source and `users/manna-harbour_miryoku/readme.org`.
- The QMK CLI is repo-versioned: `userspace-*` subcommands only exist while
  `QMK_HOME` points at the modern submodule.
- Before claiming a change works, run both (a) and (c) above. Firmware must stay
  under 28672 bytes — the build prints the size. **There is no way to test
  on-hardware behaviour here** (no `dfu-programmer`); state that explicitly and
  let the user flash.
