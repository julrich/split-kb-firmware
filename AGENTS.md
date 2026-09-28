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

Prereqs: `qmk` CLI (`python3 -m pip install qmk`), `avr-gcc`, and QMK's AVR
submodules:

```sh
git submodule update --init --depth 1 qmk_firmware
cd qmk_firmware && git submodule update --init --depth 1 lib/lufa lib/printf
```

```sh
# a) no global config touched — preferred in this environment
QMK_HOME="$(realpath qmk_firmware)" QMK_USERSPACE="$(realpath .)" \
  qmk compile -kb sofle/rev1 -km miryoku

# b) project Makefile (add SKIP_GIT=1 to stop QMK from pulling unused submodules)
make sofle/rev1:miryoku SKIP_GIT=1

# c) all qmk.json targets, hex lands in this directory
QMK_HOME="$(realpath qmk_firmware)" QMK_USERSPACE="$(realpath .)" qmk userspace-compile
```

With `qmk config user.overlay_dir="$(realpath .)"` set once, plain
`qmk compile -kb sofle/rev1 -km miryoku` works from anywhere.

Current size: **26890 / 28672 bytes (93%, 1782 free)** on `atmega32u4`. That is
the binding constraint — see §5 before adding features.

## 3. What is in the keymap (and why)

`keymap.c` contains **only** Sofle-specific hardware support; the layers come
from the vendored Miryoku tree via `INTROSPECTION_KEYMAP_C` in
`users/manna-harbour_miryoku/rules.mk`.

| feature | implementation |
|---|---|
| RGB lighting layers | `layer_miryoku_lights` (one segment set: layer-id LED, inner/outer columns, thumb clusters, all `HSV_BLUE`), installed in `keyboard_post_init_user()`; LED ids use the `N` / `35 + N` left/right half idiom |
| OLED (master) | `print_status_narrow()`, rotated 270°: small logo, `TSNM`, compiled alphas, active layer name |
| OLED (slave) | `render_logo()`, 128x32 bitmap, default rotation |
| Encoders | `encoder_update_user()`: index 0 = volume, index 1 = `MS_WHLU`/`MS_WHLD` |

Two deliberate cleanups vs. the old fork:

- The master OLED's layer names are **generated from `MIRYOKU_LAYER_LIST`**, so
  they cover all ten Miryoku layers and follow `custom_config.h` and build
  options automatically; the old hand-written `switch` only knew 8 layers.
  The alphas label (`CmkDH`) is derived from `MIRYOKU_ALPHAS_*` at compile time
  instead of being hardcoded.
- `HSV_KSDSPRI` / `HSV_KSDSSEC`, which the old fork patched into
  `quantum/color.h`, were never referenced by any code and are not carried over.
  Core files cannot be patched from a userspace — if you need custom colours,
  define them in this keymap's `config.h` (or `custom_config.h`).

## 4. Flashing

```sh
qmk flash -kb sofle/rev1 -km miryoku     # needs dfu-programmer
```

`BOOTLOADER = atmel-dfu` is set in the keymap's `rules.mk`; keymap `rules.mk` is
included after the keyboard's generated rules, so it overrides the Sofle's stock
`caterina` (verified: `-DBOOTLOADER_ATMEL_DFU` in the build flags).

`MASTER_LEFT`, no `EE_HANDS` → flash **both** halves with the same hex
(double-tap reset to enter DFU).

## 5. Conventions / traps

- Keymap directory names must match `[a-z0-9_]+`; the Miryoku user directory is
  linked by `USER_NAME = manna-harbour_miryoku` in the keymap `rules.mk`, not by
  name.
- All 72 WS2812 LEDs (36/side) are driven by **RGBLIGHT**; `RGB_MATRIX_ENABLE`
  must stay off (it is dead weight, and `RGB_MATRIX_DRIVER = WS2812` no longer
  exists — the valid value is `ws2812`).
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
