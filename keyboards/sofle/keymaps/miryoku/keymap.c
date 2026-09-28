// Copyright 2022 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

// Sofle rev1 hardware support for Miryoku: RGB lighting layers, both OLEDs and
// the two rotary encoders. The layers themselves live in
// users/manna-harbour_miryoku (INTROSPECTION_KEYMAP_C); the Sofle key mapping
// is in config.h.

#include QMK_KEYBOARD_H
#include "manna-harbour_miryoku.h"

// --------------------------------------------------------------------- RGB --

#define INDICATOR_BRIGHTNESS 30

#define HSV_OVERRIDE_HELP(h, s, v, Override) h, s, Override
#define HSV_OVERRIDE(hsv, Override) HSV_OVERRIDE_HELP(hsv, Override)

// LED clusters. Sofle rev1 has 36 LEDs per half; index N and 35 + N address the
// same position on the left and right half.
#define SET_INNER_COL(hsv)     {33, 4, hsv}, {35 + 33, 4, hsv}
#define SET_OUTER_COL(hsv)     {7, 4, hsv}, {35 + 7, 4, hsv}
#define SET_THUMB_CLUSTER(hsv) {25, 2, hsv}, {35 + 25, 2, hsv}
#define SET_LAYER_ID(hsv) \
    {0, 1, HSV_OVERRIDE_HELP(hsv, INDICATOR_BRIGHTNESS)}, \
    {35 + 0, 1, HSV_OVERRIDE_HELP(hsv, INDICATOR_BRIGHTNESS)}, \
    {1, 6, hsv}, {35 + 1, 6, hsv}, \
    {7, 4, hsv}, {35 + 7, 4, hsv}, \
    {25, 2, hsv}, {35 + 25, 2, hsv}

#ifdef RGBLIGHT_ENABLE

const rgblight_segment_t PROGMEM layer_miryoku_lights[] = RGBLIGHT_LAYER_SEGMENTS(
    SET_LAYER_ID(HSV_BLUE),
    SET_INNER_COL(HSV_BLUE),
    SET_OUTER_COL(HSV_BLUE),
    SET_THUMB_CLUSTER(HSV_BLUE));

const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
    layer_miryoku_lights);

layer_state_t layer_state_set_user(layer_state_t state) {
    rgblight_set_layer_state(0, true);
    return state;
}

void keyboard_post_init_user(void) {
    rgblight_layers = my_rgb_layers;
    rgblight_enable();
}

#endif

// --------------------------------------------------------------------- OLED --

#ifdef OLED_ENABLE

// Which alphas the firmware was built with (see post_rules.mk); Miryoku's
// default is Colemak-DH.
#if defined(MIRYOKU_ALPHAS_AZERTY)
#    define ALPHAS_NAME "AZRTY"
#elif defined(MIRYOKU_ALPHAS_BEAKL15)
#    define ALPHAS_NAME "BEAKL"
#elif defined(MIRYOKU_ALPHAS_COLEMAK)
#    define ALPHAS_NAME "Cmk"
#elif defined(MIRYOKU_ALPHAS_COLEMAKDH)
#    define ALPHAS_NAME "CmkDH"
#elif defined(MIRYOKU_ALPHAS_COLEMAKDHK)
#    define ALPHAS_NAME "CmkDHk"
#elif defined(MIRYOKU_ALPHAS_DVORAK)
#    define ALPHAS_NAME "Dvork"
#elif defined(MIRYOKU_ALPHAS_HALMAK)
#    define ALPHAS_NAME "Halmk"
#elif defined(MIRYOKU_ALPHAS_WORKMAN)
#    define ALPHAS_NAME "Wrkmn"
#elif defined(MIRYOKU_ALPHAS_QWERTY)
#    define ALPHAS_NAME "Qwrt"
#elif defined(MIRYOKU_ALPHAS_QWERTZ)
#    define ALPHAS_NAME "Qwrtz"
#else
#    define ALPHAS_NAME "CmkDH"
#endif

// Slave half: logo.
static void render_logo(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,255,255,255,255,255,  7,  7, 15,255,255,255,255,127,127,127,255,255,255,255,255,255,  0,  0,  0,  0,  0,  0,  0,  0,128,192,192,128,  0,  0,  0,  0,  0,  0,128,192,192,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,192,192,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,128,  0,  0,  0,  0,128,128,128,128,128,128,128,128,  0,  0,  0,  0,  0,  0,128,128,128,128,128,128,128,  0,
        0,255,255,255,255,255,  0,  0,  0,247,243,241,216,156, 14, 31, 63,127,255,255,255,255,  0,  0,  0,  0,  0,  0,  0,  0,255,255,255,255,240,248,252,156, 12,  0,253,253,253,253,  0,240,248,252, 28, 28, 28,188,184,176,  0,255,255,255,240,248,252,220, 12,  4,176,248,252,252,252,252,220,216, 28, 28,255,255,255,156, 28, 28,  0,240,248,252, 28, 28, 28,252,252,252,  0,252,252,252,252, 60, 28, 28, 28,255,255,255,255, 28, 28,  0,255,255,255,255,  3,  3,131,135,255,255,254,120,128,223,255,191,
        59,123,123,247,247,231,192, 31, 31, 31, 31, 31, 28, 28, 28, 31, 31, 31, 31, 31, 31, 30, 28, 30, 31, 31, 31, 31,  0,  0,  0,  0,  0,  0,  0,  0,  7,  7,  7,  7,  0,  3,  7,  7,  7,  4,  3,  7,  7,  7,  0,  1,  3,  7,  7,  7,  7,  7,  3,  0,  0,  7,  7,  7,  1,  1,  7,  7,  7,  6,  1,  3,  7,  7,  7,  7,  7,  3,  0,  0,  1,  3,  7,  7,  7,  7,  0,  1,  3,  7,  7,  7,  7,  7,  7,  7,  0,  7,  7,  7,  7,  0,  0,  0,  0,  1,  3,  7,  7,  7,  7,  0,  7,  7,  7,  7,  7,  7,  7,  7,  3,  1,  0,  0,  0,  3,  3,  7,
        7,  7,  7,  7,  3,  3,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

// Master half: rotation mark, compiled alphas and the active layer. The layer
// names come from Miryoku's own layer list, so they follow custom_config.h and
// the build options instead of drifting.
static void print_status_narrow(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,128,192,224,240,248,124, 62, 31, 31, 62,124,248,240,224,192,128,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224,240,248,124, 62,127,255,255,227,193,128,  0, 28, 62,124,248,240,224,225,227,255,255,127, 62,124,248,240,224,192,128,  1,  3,  7, 15, 31, 62,124,252,254,255,207,135, 15, 31, 62,124,120, 48,  1,  3,135,207,255,254,252,124, 62, 31, 15,  7,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  7, 15, 31, 62,124,248,248,124, 62, 31, 15,  7,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));

    oled_set_cursor(0, 5);

    oled_write_ln_P(PSTR("TSNM"), false);
    oled_write_P(PSTR("\n\n"), false);

    oled_write_ln_P(PSTR(ALPHAS_NAME), false);
    oled_write_P(PSTR("\n\n"), false);

    oled_write_ln_P(PSTR("LAYER"), false);
    switch (get_highest_layer(layer_state | default_layer_state)) {
#define MIRYOKU_X(LAYER, STRING) \
        case U_##LAYER: \
            oled_write_ln_P(PSTR(STRING), false); \
            break;
        MIRYOKU_LAYER_LIST
#undef MIRYOKU_X
        default:
            oled_write_ln_P(PSTR("Undef"), false);
            break;
    }
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    }
    return rotation;
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        print_status_narrow();
    } else {
        render_logo();
    }
    return false;
}

#endif

// ------------------------------------------------------------------ ENCODER --

#ifdef ENCODER_ENABLE

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    } else if (index == 1) {
        if (clockwise) {
            tap_code(MS_WHLD);
        } else {
            tap_code(MS_WHLU);
        }
    }
    return true;
}

#endif
