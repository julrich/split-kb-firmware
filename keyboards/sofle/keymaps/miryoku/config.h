// Copyright 2022 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#define USB_MAX_POWER_CONSUMPTION 100

#define XXX KC_NO

// Sofle rev1 (60 keys, LAYOUT) -> Miryoku (3x5 + 2 thumbs per hand).
// Miryoku's columns map to Sofle columns 1-5; Sofle column 0, the two
// encoder-adjacent keys and the outer thumb keys are unused (KC_NO).
#define LAYOUT_miryoku( \
      K00,  K01,  K02,  K03,  K04,                   K05,  K06,  K07,  K08,  K09, \
      K10,  K11,  K12,  K13,  K14,                   K15,  K16,  K17,  K18,  K19, \
      K20,  K21,  K22,  K23,  K24,                   K25,  K26,  K27,  K28,  K29, \
      N30,  N31,  K32,  K33,  K34,                   K35,  K36,  K37,  N38,  N39 \
) \
LAYOUT( \
XXX,  XXX,  XXX,  XXX,  XXX,  XXX,                 XXX,  XXX,  XXX,  XXX,  XXX,  XXX, \
XXX,  K00,  K01,  K02,  K03,  K04,                 K05,  K06,  K07,  K08,  K09,  XXX, \
XXX,  K10,  K11,  K12,  K13,  K14,                 K15,  K16,  K17,  K18,  K19,  XXX, \
XXX,  K20,  K21,  K22,  K23,  K24,  XXX,     XXX,  K25,  K26,  K27,  K28,  K29,  XXX, \
                XXX,  XXX,  K32,  K33,  K34,      K35,  K36,  K37,  XXX,  XXX \
)

#define MASTER_LEFT
// #define MASTER_RIGHT
// #define EE_HANDS

#define ENCODER_DIRECTION_FLIP

#define RGBLIGHT_SLEEP
#define RGBLIGHT_LAYERS

/* ws2812 RGB LED */
#define RGB_DI_PIN D3

#ifdef RGBLIGHT_ENABLE
    #undef RGBLIGHT_LED_COUNT

    // Only the static base effect is used; the animated effects cost flash and
    // are overridden by the lighting layers anyway.
    #undef RGBLIGHT_EFFECT_BREATHING
    #undef RGBLIGHT_EFFECT_RAINBOW_MOOD
    #undef RGBLIGHT_EFFECT_RAINBOW_SWIRL
    #undef RGBLIGHT_EFFECT_SNAKE
    #undef RGBLIGHT_EFFECT_KNIGHT
    #undef RGBLIGHT_EFFECT_CHRISTMAS
    #undef RGBLIGHT_EFFECT_STATIC_GRADIENT
    #undef RGBLIGHT_EFFECT_RGB_TEST
    #undef RGBLIGHT_EFFECT_ALTERNATING
    #undef RGBLIGHT_EFFECT_TWINKLE

    #define RGBLIGHT_LED_COUNT 72
    #define RGBLED_SPLIT { 36, 36 }
    #define RGBLIGHT_LIMIT_VAL 120
    #define RGBLIGHT_HUE_STEP 10
    #define RGBLIGHT_SAT_STEP 17
    #define RGBLIGHT_VAL_STEP 17
#endif
