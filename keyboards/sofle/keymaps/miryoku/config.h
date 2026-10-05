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
LALT(KC_SPC),  KC_MPRV,  KC_MPLY,       KC_MNXT,       KC_MUTE,       XXX, \
KC_NUHS, K00,  K01,  K02,  K03,  K04,                 K05,  K06,  K07,  K08,  K09,  KC_LBRC, \
KC_MINS, K10,  K11,  K12,  K13,  K14,                 K15,  K16,  K17,  K18,  K19,  KC_QUOT, \
KC_NUBS, K20,  K21,  K22,  K23,  K24,  XXX,     XXX,  K25,  K26,  K27,  K28,  K29,  RM_NEXT, \
                RM_TOGG,  XXX,  K32,  K33,  K34,      K35,  K36,  K37,  XXX,  XXX \
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
// effects need. Nothing is painted statically any more (see keymap.c).
#define RGB_MATRIX_KEYPRESSES
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_REACTIVE_SIMPLE

// The pulse is the only light this keymap shows, and with the static pattern gone
// the peak is one or two LEDs, so the old 120 cap only made it hard to place.
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 200
#define RGB_MATRIX_HUE_STEP 8
#define RGB_MATRIX_SAT_STEP 8
#define RGB_MATRIX_VAL_STEP 8
#define RGB_MATRIX_SPD_STEP 10
