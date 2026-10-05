// Copyright 2022 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#define USB_MAX_POWER_CONSUMPTION 100

#define XXX KC_NO

// Sofle rev1 (60 keys, LAYOUT) -> Miryoku (3x5 + 3 thumbs per hand), German.
// Miryoku's columns map to Sofle columns 1-5; every cell it leaves over is
// filled in here, in this one macro, so those keys behave identically on all ten
// layers (a function row should not vanish while a layer is held):
//   number row  : German "´" plus the Hyprland window/app chords from
//                 ~/.config/hypr/hyprland.conf ($mainMod = SUPER) and media keys
//   left column : German "#" / "ß" / "<" (Sofle column 0)
//   right column: the umlauts "ü" above "ä"; "ö" is Miryoku's own cell one key
//                 inward (substituted in users/manna-harbour_miryoku/custom_config.h)
//   RGB control : the outermost left thumb (on/off) and the bottom-right outer
//                 key (next effect). Those must be RM_*, not UG_*: UG_* is
//                 RGBLIGHT, which this build does not use.
//
// Usages on a German host layout (QMK's keymap_german.h names them DE_ACUT etc.,
// but no QMK header can be included from here: config.h is also pulled into the
// assembly translation units, so the raw keycodes are the only option):
//   KC_EQL  ("=+")  = ´ / `     KC_NUHS ("#~")  = # / '     KC_NUBS ("<>") = < / >
//   KC_MINS ("-_")  = ß / ?     KC_LBRC ("[{")  = ü
//   KC_QUOT ("'\"") = ä         KC_SCLN (";:")  = ö
#define LAYOUT_miryoku( \
      K00,  K01,  K02,  K03,  K04,                   K05,  K06,  K07,  K08,  K09, \
      K10,  K11,  K12,  K13,  K14,                   K15,  K16,  K17,  K18,  K19, \
      K20,  K21,  K22,  K23,  K24,                   K25,  K26,  K27,  K28,  K29, \
      N30,  N31,  K32,  K33,  K34,                   K35,  K36,  K37,  N38,  N39 \
) \
LAYOUT( \
KC_EQL,  LGUI(KC_Q),      LGUI(KC_R),    LGUI(KC_E),    LGUI(KC_F),    LGUI(S(KC_Q)), \
LALT(KC_SPC),  KC_MPRV,  KC_MPLY,       KC_MNXT,       KC_MUTE,       RM_HUEU, \
KC_NUHS, K00,  K01,  K02,  K03,  K04,                 K05,  K06,  K07,  K08,  K09,  KC_LBRC, \
KC_MINS, K10,  K11,  K12,  K13,  K14,                 K15,  K16,  K17,  K18,  K19,  KC_QUOT, \
KC_NUBS, K20,  K21,  K22,  K23,  K24,  XXX,     XXX,  K25,  K26,  K27,  K28,  K29,  RM_NEXT, \
                RM_TOGG,  RM_PREV,  K32,  K33,  K34,      K35,  K36,  K37,  RM_VALD,  RM_VALU \
)

#define MASTER_LEFT
// #define MASTER_RIGHT
// #define EE_HANDS

#define ENCODER_DIRECTION_FLIP

// The WS2812 pin, the split LED counts and the per-key LED layout are declared
// by the Sofle itself (keyboards/sofle/info.json: ws2812.pin, rgb_matrix.layout),
// so nothing about the strip needs to be repeated here.

// Per-key LEDs are driven by RGB Matrix rather than RGBLIGHT: the keyboard's
// own LED layout gives every key a position, which is what the keypress-reactive
// effects need. The static accent pattern is painted in keymap.c.
//
// Enabled effects - `RM_NEXT` cycles exactly these (plus SOLID_COLOR, which QMK
// always compiles) in this order, and `RM_PREV` walks back. Marginal flash cost
// measured 2026-10-05 (~/effect-costs.txt); the whole set is what fits in the
// budget, not the budget being generous:
//   SOLID_REACTIVE_SIMPLE       the default: pressed keys flash and fade
//   SOLID_REACTIVE              every LED lit, each press punches a fading hole
//   SOLID_REACTIVE_WIDE         +462 B: the splash runner, a wide glow per press
//   SPLASH / SOLID_SPLASH       +60/+58 B: a splash that spreads out
//   MULTISPLASH                 +10 B: several splashes at once
//   SOLID_MULTISPLASH           +8 B: and on the fully lit variant
//   SOLID_REACTIVE_MULTIWIDE    +8 B: several wide glows at once
//   CYCLE_LEFT_RIGHT            +28 B: a moving rainbow, the cheap classic
// (NEXUS would add a converging splash for +116 B, CYCLE_UP_DOWN another +62 B;
// neither fits alongside the above, so both are left out.)
#define RGB_MATRIX_KEYPRESSES
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
#define ENABLE_RGB_MATRIX_SPLASH
#define ENABLE_RGB_MATRIX_SOLID_SPLASH
#define ENABLE_RGB_MATRIX_MULTISPLASH
#define ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
#define ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_REACTIVE_SIMPLE

// The pulse is the main light this keymap shows, and with the static pattern
// confined to the accents the peak is one or two LEDs, so the old 120 cap only
// made it hard to place. `val` is runtime-adjustable and lives in EEPROM, so this
// is only the value a fresh EEPROM gets.
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 200
// Slower fade than QMK's default (127 = ~0.5 s): 32 gives ~2 s on the reactive
// effects. Also runtime-adjustable, also only the fresh-EEPROM value.
#define RGB_MATRIX_DEFAULT_SPD 32
#define RGB_MATRIX_HUE_STEP 8
#define RGB_MATRIX_SAT_STEP 8
#define RGB_MATRIX_VAL_STEP 8
#define RGB_MATRIX_SPD_STEP 10
