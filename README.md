# qmk_userspace — Sofle rev1 / Miryoku

External QMK userspace for a **Sofle rev1** split keyboard running
[Miryoku](https://github.com/manna-harbour/miryoku), with personal hardware
customisations layered on top. See [AGENTS.md](AGENTS.md) for the working notes.

```
keyboards/sofle/keymaps/miryoku/     Sofle keymap: layout mapping + hardware support
users/manna-harbour_miryoku/         vendored Miryoku user space (layer definitions)
qmk.json                             build targets for `qmk userspace-compile` / CI
qmk_firmware/                        git submodule, pinned QMK commit
```

## Pins

| component | version |
|---|---|
| QMK firmware | submodule, `b1aea2556040c28d58d611c8aef0082b5780bb6d` (master, 2026-09-26) |
| Miryoku QMK | vendored from [yehorb/qmk_userspace](https://github.com/yehorb/qmk_userspace/tree/miryoku) @ `2a827f9d0667fb54f355f36ab16b6f76624841a8` (2026-05-27), unmodified |

There is no upstream-maintained Miryoku userspace port yet; the vendored tree is
the current working port (see [miryoku#287](https://github.com/manna-harbour/miryoku/discussions/287)).
When re-vendoring, diff `users/manna-harbour_miryoku/` against upstream and record
the new commit here.

Update QMK deliberately:

```sh
cd qmk_firmware && git fetch --depth 1 origin master && git checkout FETCH_HEAD
cd .. && git add qmk_firmware && git commit -m "chore: bump QMK to <sha>"
```

## Build

Prerequisites: the `qmk` CLI (installed with pipx, see below), `avr-gcc`, and the
submodules QMK needs for AVR:

```sh
pipx install qmk
pipx inject qmk -r "$(realpath qmk_firmware)/requirements.txt" appdirs

git submodule update --init --depth 1 qmk_firmware
cd qmk_firmware && git submodule update --init --depth 1 lib/lufa lib/printf
```

### Local (recommended)

```sh
QMK_HOME="$(realpath qmk_firmware)" QMK_USERSPACE="$(realpath .)" \
  qmk compile -kb sofle/rev1 -km miryoku
QMK_HOME="$(realpath qmk_firmware)" QMK_USERSPACE="$(realpath .)" \
  qmk compile -kb sofle/rev1 -km miryoku -e MIRYOKU_ALPHAS=QWERTY
```

Both variables are set explicitly on purpose: while the old fork still lives at
`~/qmk_firmware`, a bare `qmk compile` resolves `QMK_HOME` to *that* tree.

One-off alternative, if you no longer need to build the old fork:

```sh
qmk config user.qmk_home="$(realpath qmk_firmware)" user.overlay_dir="$(realpath .)"
qmk compile -kb sofle/rev1 -km miryoku
```

### Make wrapper

```sh
make sofle/rev1:miryoku            # forwards to the pinned qmk_firmware
make sofle/rev1:miryoku:flash
make sofle/rev1:miryoku SKIP_GIT=1 # skip QMK's submodule check (no surprise downloads)
```

### All targets / CI

```sh
qmk userspace-compile      # builds everything in qmk.json
```

Pushing to GitHub runs `.github/workflows/build_binaries.yaml`, which builds all
`qmk.json` targets **against the pinned submodule** (the reusable workflow skips
its own `qmk_firmware` checkout when the submodule is present) and publishes a
Release with the hex files. Actions is enabled by default — the first run
happens on the first push.

## Flashing

Both halves run the **same** firmware (`MASTER_LEFT`, no `EE_HANDS`) but are
separate MCUs, so each is flashed over its own USB port, one at a time:

```sh
make sofle/rev1:miryoku:flash SKIP_GIT=1     # per half, from this directory
```

The build runs first, then `dfu-programmer` looks for a board in DFU mode and, if
it finds none, **waits — retrying every 0.5 s**. So the easy workflow is: start
the command, then enter the bootloader on that half while it waits. Three ways
in:

1. **Double-tap the reset button** on that half's Pro Micro (next to the TRRS
   jack) — two quick presses; a single press only restarts the MCU.
2. **Hold the outermost-top key of that half while plugging in its USB** —
   Bootmagic Lite. It also clears the stored EEPROM config, which is handy after
   a firmware switch.
3. **`QK_BOOT`** from the layout (Miryoku has it behind a double tap on the
   additional-features key).

Then move the USB cable to the other half, enter *its* bootloader and flash the
same command. Success = the half re-enumerates as `fc32:0287 JosefAdamcik Sofle`
(`lsusb`) instead of the bootloader's `03eb:2ff4 Atmel`.

Never plug or unplug the TRRS cable while USB is connected. The bootloader is
`atmel-dfu` (from the keymap's `rules.mk`; the stock Sofle declares `caterina`);
`dfu-programmer` is required. Full detail: [AGENTS.md §5](AGENTS.md).

## Keymap

![Sofle rev1 / Miryoku layout with this board's caps](docs/sofle-layout.png)

Every key: bold = what it sends (base layer, German host, decoded from the
*compiled* keymap, tap-hold layers included); grey = the **key number**:
each half counted from its inner edge outward and top to bottom — `L1`–`L6` /
`R1`–`R6` the number row, six per row after that, `L25`–`L29` / `R25`–`R29` the
thumbs (the wide inner one first). The tables below refer to keys by these
numbers, so any cap legends on the board are irrelevant to them.

Regenerate after a keymap change with:

```sh
python3 tools/gen_layout_svg.py docs/sofle-layout.svg
rsvg-convert -z 1.6 docs/sofle-layout.svg -o docs/sofle-layout.png
```

### Layers

Miryoku's ten layers, unchanged except for the German substitutions in
`users/manna-harbour_miryoku/custom_config.h`. Reached by holding a thumb (tap =
the small function):

| hold | key | tap |
|---|---|---|
| Media | `L27` (left, outer Miryoku thumb) | Esc |
| Nav | `L26` (left, middle) | Space |
| Mouse | `L28` (left, next to the outer thumbs) | Tab |
| Sym | `R25` (right, inner, 1.5u) | Enter |
| Num | `R26` (right, middle) | Backspace |
| Fun | `R27` (right, outer) | Delete |
| Button | `L23` and `R23` — in the bottom row, one key in from the outer column | that key's output |

Layer contents, options and Miryoku's own diagrams are in
`users/manna-harbour_miryoku/readme.org`.

### The 24 keys beyond Miryoku's 36

All of them live in `LAYOUT_miryoku` in the keymap's `config.h` and behave the
same on **every** layer, so the numbers stay put while a layer is held.

| key | position | sends | notes |
|---|---|---|---|
| `L6` | left column, number row | `KC_EQL` | `´`, `Shift` = `` ` `` |
| `L12` | left column, Q row | `KC_NUHS` | `#`, `Shift` = `'` |
| `L18` | left column, A row | `KC_MINS` | `ß`, `Shift` = `?` |
| `L24` | left column, Z row | `KC_NUBS` | `<`, `Shift` = `>` |
| `L1`–`L5` | left number row | `LGUI` + `Q`/`R`/`E`/`F`, `LGUI`+`Shift`+`Q` | kitty, wofi, dolphin, fullscreen, close window |
| `R1` | right number row, inner | `LAlt` + `Space` | tmux special workspace |
| `R2`–`R5` | right number row | media | prev track, play/pause, next track, mute |
| `R12`, `R18` | right column, Q + A rows | `KC_LBRC` / `KC_QUOT` | `ü`, `ä` |
| `L29` | outermost left thumb | `RM_TOGG` | RGB Matrix on/off |
| `L25` | left inner wide thumb | `KC_F3` | while held: fievel Free Mouse Mode; overrides Miryoku's Mouse/Tab thumb |
| `L28` | left thumb next to `RM_TOGG` | `LT(Mouse, KC_TAB)` | built-in Mouse layer (hold) and Tab (tap) — the fallback |
| `R24` | bottom-right outer key | `RM_NEXT` | next RGB effect (cycles eight modes) |
| `R6` | outermost right number row | `RM_HUEU` | hue, wraps |
| `R29`, `R28` | the two outer right thumbs | `RM_VALU`, `RM_VALD` | brighter / dimmer; also dims the blue accents |

The `ö` cell is Miryoku's own (substituted in `custom_config.h`), and `ß` also
stays where Miryoku keeps it, on the Num layer. The four spare cells Miryoku
leaves are used up by these RGB controls; the only cells with nothing on them are
the two knob push-buttons, which this board does not wire to the matrix.

### Encoders, lighting, OLED

| feature | behaviour |
|---|---|
| Left encoder | volume up/down |
| Right encoder | PgUp/PgDn on Base/Extra/Tap, Up/Down while Nav is held, mouse wheel on the other layers |
| RGB presets | eight modes — `SOLID_COLOR`, `SOLID_REACTIVE_SIMPLE` (default), `SOLID_REACTIVE`, `SOLID_REACTIVE_WIDE`, `SOLID_REACTIVE_MULTIWIDE`, `SPLASH`, `MULTISPLASH`, `SOLID_MULTISPLASH` — cycled with `RM_NEXT`; mode, hue, brightness and fade live in EEPROM and survive a reboot |
| Blue accents | static: the off-grid ring, the outer column, the inner column and the thumbs of each half; a keypress pulse shows through them. The two knob LEDs (indices 0 and 36) are dimmer and are the only always-on light |
| OLED | the half holding the USB cable shows TSNM / compiled alphas / live layer; the other half shows the logo |
| LED table | this board's cell→LED order does not match the keyboard definition; it is replaced by a measured table (`led_table.h`) — see `AGENTS.md` §6 |

Firmware size is **28642/28672 bytes** (30 B free) and RAM 2036/2560. Adding an
effect costs 26–290 B depending on what code it shares, so measure first —
`docs/effect-costs.txt` has every candidate's measured cost.

### Host-side mouse replacement (fievel)

A HID keyboard has no channel *from* the host, so the firmware cannot detect
whether a host-side tool is running — "use fievel if present" has to be a
placement decision, not an automatic switch. The split here:

| key | while held | on tap |
|---|---|---|
| `L25`, left inner wide thumb | sends **`F3`** — [fievel](https://github.com/MontyTheSoftwareEngineer/fievel) in its default *hold* mode treats that as Free Mouse Mode | `F3` (harmless) |
| `L28`, left thumb next to `RM_TOGG` | the built-in **Mouse layer** (`LT(Mouse, KC_TAB)`) — works on any host: BIOS, installers, login screens, VMs | Tab |

Fievel's own defaults need no changes: `free_mouse = "f3"` and `mode = "hold"`
are the shipped settings. Do **not** enable its remapper / home-row mods — ours
are resolved in the firmware, before evdev, and work everywhere; enabling both
would double-remap. Fievel's hint mode (click anything by typing its label) has
no firmware equivalent and is the main reason to run it.

### Alphas and options

The schematic and every table here identify keys by **number**, never by cap
legend, so whatever is on the board stays consistent with this document.

Alphas are **QWERTY** by default (`MIRYOKU_ALPHAS = QWERTY` in the keymap's
`rules.mk`, so local builds and CI agree). That is deliberate even for a German
keyboard: **HID usages are positions, and the German host layout performs the
Z/Y swap itself.** `MIRYOKU_ALPHAS=QWERTZ` moves the usages to where a German
keyboard has them, so on a German host the two swaps cancel and every key types
its US legend. QWERTZ alphas is for a *US* host. Net effect with QWERTY alphas
and the `de` layout: the key labelled `y` types `z`, the key labelled `z` types
`y`.

**German umlauts** are placed where a German keyboard has them: `KC_SCLN` (Miryoku's
own cell next to `L`) types `ö`, and the two outermost right-hand keys carry
`KC_LBRC` and `KC_QUOT`, i.e. `ü` above `ä`.

Any Miryoku option can be overridden per build, e.g.
`make sofle/rev1:miryoku MIRYOKU_ALPHAS=COLEMAKDH` or
`qmk compile -kb sofle/rev1 -km miryoku -e MIRYOKU_NAV=INVERTEDT`.

Capabilities the old personal keymap had and this one does not (numpad layer,
runtime layout switching, tri-layer, `EE_CLR`, brightness, suspend, Discord mute,
per-layer RGB colours) are listed in
[AGENTS.md §4](AGENTS.md#4-not-here-yet-what-the-old-julrich-keymap-had).
