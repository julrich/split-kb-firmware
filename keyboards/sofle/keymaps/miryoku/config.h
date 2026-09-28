// Copyright 2022 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#define USB_MAX_POWER_CONSUMPTION 100

#define XXX KC_NO

// Sofle rev1 (60 keys, LAYOUT) -> Miryoku (3x5 + 3 thumbs per hand), German.
// Miryoku's columns map to Sofle columns 1-5; Sofle column 0, the number row,
// the two encoder-adjacent keys and the outer thumb keys are unused (KC_NO) --
// except the two outermost right-hand keys, which carry the umlauts a German
// layout puts there. "ü" above "ä", both one key outward from the "ö" next to L
// (that cell is Miryoku's own, substituted in custom_config.h).
//
// Usages on a German host layout (QMK's keymap_german.h names them DE_UDIA etc.,
// but that header cannot be included from here: config.h is also pulled into the
// assembly translation units; see the DE_* aliases there if you need them):
//   KC_LBRC ("[{")  = ü        KC_QUOT ("'\"")  = ä        KC_SCLN (";:") = ö
//   KC_MINS ("-_")  = ß  (Miryoku keeps that one on the Num layer)
#define LAYOUT_miryoku( \
      K00,  K01,  K02,  K03,  K04,                   K05,  K06,  K07,  K08,  K09, \
      K10,  K11,  K12,  K13,  K14,                   K15,  K16,  K17,  K18,  K19, \
      K20,  K21,  K22,  K23,  K24,                   K25,  K26,  K27,  K28,  K29, \
      N30,  N31,  K32,  K33,  K34,                   K35,  K36,  K37,  N38,  N39 \
) \
LAYOUT( \
XXX,  XXX,  XXX,  XXX,  XXX,  XXX,                 XXX,  XXX,  XXX,  XXX,  XXX,  XXX, \
XXX,  K00,  K01,  K02,  K03,  K04,                 K05,  K06,  K07,  K08,  K09,  KC_LBRC, \
XXX,  K10,  K11,  K12,  K13,  K14,                 K15,  K16,  K17,  K18,  K19,  KC_QUOT, \
XXX,  K20,  K21,  K22,  K23,  K24,  XXX,     XXX,  K25,  K26,  K27,  K28,  K29,  XXX, \
                XXX,  XXX,  K32,  K33,  K34,      K35,  K36,  K37,  XXX,  XXX \
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
// effects need. The static blue accents are drawn in keymap.c.
#define RGB_MATRIX_KEYPRESSES
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_REACTIVE_SIMPLE

#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 120
#define RGB_MATRIX_HUE_STEP 8
#define RGB_MATRIX_SAT_STEP 8
#define RGB_MATRIX_VAL_STEP 8
#define RGB_MATRIX_SPD_STEP 10
