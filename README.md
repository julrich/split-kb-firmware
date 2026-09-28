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

`qmk flash -kb sofle/rev1 -km miryoku` — needs `dfu-programmer`
(`sudo pacman -S dfu-programmer`) and the board in DFU mode (double-tap reset).
The bootloader is overridden to `atmel-dfu` from the keymap's `rules.mk` (the
stock Sofle declares `caterina`). Flash both halves; `MASTER_LEFT`.

Lighting is RGB Matrix on the Sofle's own LED layout: pressed keys pulse
(`RGB_MATRIX_SOLID_REACTIVE_SIMPLE`) and the static blue accents are drawn as
LED indicators. Firmware size is 27654/28672 bytes.

## Layout

- `keyboards/sofle/keymaps/miryoku/config.h` — Sofle→Miryoku key mapping and
  lighting config
- `keyboards/sofle/keymaps/miryoku/keymap.c` — RGB accents, both OLEDs, encoders
- `users/manna-harbour_miryoku/` — vendored Miryoku (layers, options, docs)
