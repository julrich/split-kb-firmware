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
the binding constraint — check the number after every change, see §6.

## 3. What is in the keymap (and why)

`keymap.c` contains **only** Sofle-specific hardware support; the layers come
from the vendored Miryoku tree via `INTROSPECTION_KEYMAP_C` in
`users/manna-harbour_miryoku/rules.mk`.

| feature | implementation |
|---|---|
| Per-key LEDs | **RGB Matrix** with the Sofle's own LED layout (72 LEDs, 58 under keys, in `keyboards/sofle/info.json`). Default mode `RGB_MATRIX_SOLID_REACTIVE_SIMPLE` — the key you press pulses and fades. Static blue accents are drawn in `rgb_matrix_indicators_advanced_user()` from `accent_leds[]` |
| OLED (master) | `print_status_narrow()`, rotated 270°: small logo, `TSNM`, the compiled alphas (`Qwrtz` by default, see `rules.mk`), active layer name |
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

## 4. Not here (yet): what the old `julrich` keymap had

Miryoku maps **36 of the Sofle's 60 keys**. Still unused: the whole number row
(12 keys), the four outermost keys of the left hand, the two encoder-adjacent
keys, and the four outer thumb keys. (The right hand's outermost column is no
longer unused — its top and home keys now carry `ü` and `ä`, see §6.) Numbers
live on Miryoku's Num layer. Whatever free keys remain are where anything below
would go.

| julrich capability | old implementation | now |
|---|---|---|
| Numpad layer | `_NUMPAD` on `LT(_NUMPAD, KC_TAB)`: `KC_P0`-`KC_P9`, `KC_NUM`, `OSM(MOD_MEH)` | absent |
| Runtime layout switching | `KC_QWERTY`/`KC_COLEMAK`/`KC_COLEMAKDH`/`KC_MIRYOKU` → `set_single_persistent_default_layer()` | absent — Miryoku picks alphas at build time (`MIRYOKU_ALPHAS=`) |
| Tri-layer | `update_tri_layer(_LOWER, _RAISE, _ADJUST)` | absent |
| Discord mute | `KC_D_MUTE` (MEH+Up) on the encoder-adjacent key | absent, key unmapped |
| EEPROM reset | `EE_CLR` on `_ADJUST`/`_SWITCH` | absent — use Bootmagic at power-up (§5) |
| Screen brightness | `KC_BRIU` on `_SWITCH` | absent |
| Suspend | `KC_SYSTEM_SLEEP` on `_SWITCH` | absent |
| Desk switching | `C(G(KC_LEFT))` / `C(G(KC_RGHT))` on `_ADJUST` | absent |
| Layer jumps | `_SWITCH` overlay with `TO(0)`…`TO(6)` | absent |
| Per-layer RGB colours | 7 RGBLIGHT lighting layers (red/pink/teal/blue/purple/orange/green) | replaced by the static blue accents + reactive pulse |
| OLED brand line | `Dane Evans` + default-layer name | replaced by `TSNM` + compiled alphas + live layer |
| Encoder 1 per layer | PGUP/PGDN on base, UP/DOWN on lower/raise, wheel elsewhere | wheel only (unchanged from the old Sofle-Miryoku keymap) |

Carried over from `julrich`: `MASTER_LEFT`, `ENCODER_DIRECTION_FLIP`,
`USB_MAX_POWER_CONSUMPTION 100`, the RGB brightness cap, tap-hold behaviour
(`QUICK_TAP_TERM 0`), and — via Miryoku itself — `QK_BOOT` (double-tap the
additional-features key), media transport + mute (Media layer) and RGB
toggle/hue/sat/val/next (Media layer, `UG_*`).

Re-adding any of these: `users/manna-harbour_miryoku/custom_config.h` is
Miryoku's supported place to substitute/add layers and per-layer mappings — use
it instead of editing the vendored tree. Keycode-level extras (brightness,
suspend, `EE_CLR`, numpad) fit there or in this keymap's `config.h`/`keymap.c`.

## 5. Flashing — exact process

`atmel-dfu`, `MASTER_LEFT`, no `EE_HANDS` ⇒ **both halves run the identical
firmware**; they are two independent MCUs, so each is flashed over its own USB
port, one at a time.

```sh
# run from the userspace root, once per half:
make sofle/rev1:miryoku:flash SKIP_GIT=1
```

`dfu-programmer` (installed; `dfu-programmer 1.1.0`) and the udev rules
(`/etc/udev/rules.d/50-qmk.rules`, Jan 2023) are the only host prerequisites.
`BOOTLOADER = atmel-dfu` comes from the keymap's `rules.mk`, which is included
after the keyboard's generated rules and therefore overrides the Sofle's stock
`caterina` (verified: `-DBOOTLOADER_ATMEL_DFU` in the build flags).

The command builds first, then hands over to `dfu-programmer` — and if no device
is in DFU mode it **waits and retries every 0.5 s** (observed: `Bootloader not
found. Make sure the board is in bootloader mode.` / `Trying again every 0.5s
(Ctrl+C to cancel)`). So the comfortable order is: start the command, then enter
the bootloader on that half while it is waiting; it picks the board up and
flashes. Ctrl+C cancels. (A full flash of both halves takes well under a minute
once the bootloader is up.)

### Entering the bootloader — three ways

1. **Reset button, twice, quickly.** The Pro Micro's reset button sits next to
   the TRRS jack. A single press only restarts the MCU; the *double* tap lands in
   DFU (the upstream Sofle readme puts it as "briefly press the button near the
   TRRS connector — quickly double-tap if you are using Pro Micro"). This is the
   "double-tap reset" the QMK/Sofle docs mean.
2. **Bootmagic Lite: hold the matrix (0,0) key while plugging the half into
   USB.** On the left half that is its top-left (outermost-top) key — `Esc` in
   the old keymaps, unmapped in Miryoku, which does not matter because Bootmagic
   reads the matrix rather than the keymap; on the right half, the equivalent
   outermost-top key of that half. `bootmagic_scan()` also calls
   `eeconfig_disable()`, so this route **wipes the stored EEPROM config** as
   well — the clean slate if settings ever look wrong.
3. **Keycode.** Miryoku puts `QK_BOOT` behind a double tap on the
   additional-features key. The old `julrich` firmware had it plainly on
   `_SWITCH`/`_ADJUST`, so if a half still runs that firmware this works right
   now.

### Both halves

Observed success (left half, 2026-09-28):

```
Flashing for bootloader: atmel-dfu
Bootloader Version: 0x00 (0)
Checking memory from 0x0 to 0x6FFF...  Not blank at 0x1.
Erasing flash...  Success
Programming 0x6C80 bytes...
Success
Reading 0x7000 bytes...
Success
Validating...  Success
0x6C80 bytes written into 0x7000 bytes memory (96.88%).
```

```sh
# 1. left half (master) — host cable normally lives here
#    double-tap its reset button (or hold its top-left key while plugging USB in)
make sofle/rev1:miryoku:flash SKIP_GIT=1

# 2. right half (slave) — move the USB cable to the right half
#    double-tap *its* reset button
make sofle/rev1:miryoku:flash SKIP_GIT=1
```

#### Which half did I just flash?

Both halves are **identical on USB** — same VID/PID (`fc32:0287`), no serial — so
neither `lsusb` nor the flash output says which board was written. The flash
command simply waits for *any* ATmega32u4 in DFU mode, so forgetting to move the
cable silently flashes the same half twice (happened on 2026-09-28; the left half
got the build twice and the right half kept the previous one).

The kernel log is the ground truth, and it distinguishes the two cases:

| journal signature | meaning |
|---|---|
| `fc32:0287 Sofle` → `03eb:2ff4 ATm32U4DFU` → `Sofle` | a flash (same board) |
| `fc32:0287 Sofle` → `fc32:0287 Sofle` (new device number) | **cable moved** to the other half |

```sh
journalctl -k --since "-10 min" | grep "usb [0-9-]*:" | grep -E "USB disconnect|Product:"
```

Check for the `Sofle → Sofle` transition before trusting that step 2 really
targets the other half. Device numbers increase monotonically, so a cable move
is easy to spot (e.g. device 26 → 27 at 20:46:51, then the right-half write at
20:47:09 shows 27 → `ATm32U4DFU` → 29).

Both halves then run the **same** artifact — verify with the build output, not
with `lsusb`: 77798 bytes (local `avr-gcc`) and `-DMIRYOKU_ALPHAS_QWERTY` in
`qmk_firmware/.build/obj_sofle_rev1_miryoku/cflags.txt`.

- The TRRS cable between the halves may stay connected, but **never plug or
  unplug TRRS while USB is connected** — that can kill the controllers.
- Success looks like `dfu-programmer` erasing/programming/resetting the chip,
  after which the half re-enumerates as `fc32:0287 JosefAdamcik Sofle` instead of
  the bootloader's `03eb:2ff4 Atmel`. `lsusb` is the quick check.
- Both halves must be flashed after any change: `MASTER_LEFT` means the left half
  talks to the host, but the right half renders its own LEDs/OLED and runs the
  same code.
- EEPROM: the 2022 fork used `EECONFIG_MAGIC_NUMBER` `0xFEE7`, current QMK uses
  `0xFEE3`, so the first boot on this firmware re-initialises the stored config
  by itself. If anything still behaves oddly, flash via route 2 (Bootmagic) once.

## 6. Conventions / traps

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
- Miryoku build options: the keymap's `rules.mk` sets `MIRYOKU_ALPHAS = QWERTY`
  as the default for every build (local and CI). Any option can be overridden on
  the command line, which beats the file — e.g.
  `make sofle/rev1:miryoku MIRYOKU_ALPHAS=COLEMAKDH` (verified: the override
  switches the define and the OLED label). Values are case-insensitive; the full
  option list is in `.github/workflows/test-all-configs.yml` in the Miryoku
  source and `users/manna-harbour_miryoku/readme.org`.
- **`MIRYOKU_ALPHAS=QWERTZ` is for a *US* host.** HID usages are positions; the
  host layout maps them to characters. A German host already maps the US `y`
  position (0x1C) → `z` and the US `z` position (0x1D) → `y`. Miryoku's QWERTZ
  alphas move the usages to the German positions, so on a German host the two
  swaps cancel and every key types its US legend (seen on 2026-09-28: "y still
  prints y, z still prints z"). With `MIRYOKU_ALPHAS=QWERTY` and a `de` host the
  result is German-correct: the key labelled `y` types `z`.
- German build: `users/manna-harbour_miryoku/custom_config.h` substitutes the
  base/extra/tap layers (guarded by `#if defined(MIRYOKU_ALPHAS_QWERTY)`) so the
  cell Miryoku leaves as `KC_QUOT` — which a German host renders as `ä` — sends
  `KC_SCLN`, i.e. `ö`. The `ä` and `ü` keys are the two outermost right-hand keys
  in `LAYOUT_miryoku`. Verified in the compiled array: home row reads
  `ä, ö(LGUI_T), L, K, J, H` outer-to-inner, top row `ü, P, O, I, U, Y`.
- **Never `#include` a QMK header from `config.h`.** `config.h` is pulled into the
  assembly translation units (e.g. `platforms/avr/xprintf.S`), so adding
  `keymap_german.h` (which brings in `keycodes.h`) fails with
  `Error: junk at end of line` from the assembler. Use the raw keycodes and
  document the intent, or include the header in a `.c` file.
- A German host layout makes several Miryoku keys render as German punctuation
  (the `/` key types `-`, Sym's `KC_COLN` types `Ö`, Num's `KC_MINS`/`KC_LBRC`
  type `ß`/`ü`). `'` is *not* reachable on this build — on a German layout that
  is `Shift`+`#`, and Miryoku has no `#`/NUHS key.
- The QMK CLI is repo-versioned: `userspace-*` subcommands only exist while
  `QMK_HOME` points at the modern submodule.
- Before claiming a change works, run both (a) and (c) above. Firmware must stay
  under 28672 bytes — the build prints the size. Flashing works from here
  (`dfu-programmer` is installed, §5) but the board has to be put into DFU mode by
  hand, and only **keystrokes** can be verified from here (§7); OLED content, LED
  effects and the encoders can only be confirmed by the user.

## 7. Verifying the running firmware without root

The QMK udev rules tag the board with `uaccess`, so the logged-in desktop user can
read its `hidraw` nodes — no `sudo`, no membership in `input`. The keyboard
interface is the one with `bInterfaceProtocol=1`:

```sh
for d in /sys/class/hidraw/hidraw*; do
    n=$(grep -oP '(?<=HID_NAME=).*' "$d/device/uevent" 2>/dev/null)
    case "$n" in *Sofle*)
        echo "$d iface=$(cat "$d/device/../bInterfaceNumber") proto=$(cat "$d/device/../bInterfaceProtocol")";;
    esac
done
# → /sys/class/hidraw/hidraw5 iface=0 proto=1  (boot keyboard)
# → /sys/class/hidraw/hidraw6 iface=1 proto=0  (raw HID)

timeout 60 stdbuf -oL xxd -p -c8 /dev/hidraw5   # 8 bytes: modifiers, rsvd, keycode[6]
```

Boot-protocol reports are enough to verify the matrix, the split link and the
layer logic: each press/release is one report, and the keycode tells which layer
resolved it. Consumer (volume/media), mouse and encoder reports leave over other
HID interfaces that have **no** hidraw node — those need `sudo evtest` or the
host's own OSD.

Verified 2026-09-28 with both halves on this firmware, **with the Colemak-DH
default that was in place before the QWERTY switch** (physical QWERTY keycap
labels):

| physical key | report | keycode | proves |
|---|---|---|---|
| `a` | `00 00 04 …` | `a` | base works on the left half |
| `t` | `00 00 05 …` | `b` | alphas are Colemak-DH (QWERTY `T` → `B`) |
| `n` | `00 00 0e …` | `k` | right half matrix + split link |
| `l` | `00 00 0c …` | `i` | right half matrix + split link |
| `a` + Num held | `00 00 33 …` | `;` | `LT(U_NUM, KC_BSPC)` on the right half engaged the layer, and the left half resolved `a` as `NUM` home-row col 1 (`KC_SCLN`) |

With the current **German (QWERTY alphas + German host)** build the base layer
reads `… l ö ä` outer-to-inner on the home row, so a capture of those three keys
plus the outermost top key should show HID `0x33` (ö), `0x34` (ä) and `0x2F` (ü),
while the Z/Y keys report the plain US usages — physical `y` → `0x1C`, physical
`z` → `0x1D` — and the `de` host renders those as `z` and `y`.

Thumb keys (Miryoku defaults — "inner thumb" is ambiguous, so here is the map):

| half | position | hold | tap |
|---|---|---|---|
| left | outermost of the three | Media | Esc |
| left | middle | Nav | Space |
| left | innermost, wide 1.5u | Mouse | Tab |
| right | innermost, wide 1.5u | Sym | Enter |
| right | middle | Num | Backspace |
| right | outermost | Fun | Delete |
