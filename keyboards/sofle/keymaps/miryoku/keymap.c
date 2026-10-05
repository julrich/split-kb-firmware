// Copyright 2022 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

// Sofle rev1 hardware support for Miryoku: RGB lighting layers, both OLEDs and
// the two rotary encoders. The layers themselves live in
// users/manna-harbour_miryoku (INTROSPECTION_KEYMAP_C); the Sofle key mapping
// is in config.h.

#include QMK_KEYBOARD_H
#include "manna-harbour_miryoku.h"
// Corrected matrix-cell -> LED-index table (see the file for why the keyboard's own
// is wrong); defines the strong g_led_config that overrides the generated one.
#include "led_table.h"

// --------------------------------------------------------------------- RGB --

// The static blue accent pattern (the off-grid ring, the outer column, the inner
// column and the thumbs) is `led_accents[]` in led_table.h: it is generated from
// the corrected LED table in that same file, so the pattern and the table cannot
// disagree. The two knob LEDs (indices 0 and 36) are painted dim and are the only
// permanent light; everything else waits for a keypress.
//
// An accented LED is skipped while the reactive effect has a hit on it, so a press
// on any accented key - every thumb, both columns, the ring - still shows its pulse
// on top of the accent. The old hand-written list could not: it came from the
// RGBLIGHT keymap's `35 + N` indices, lit the wrong LEDs, and hid the pulses on the
// keys it did cover.
#define ACCENT_BRIGHTNESS 90
#define INDICATOR_BRIGHTNESS 30

#ifdef RGB_MATRIX_ENABLE

// ---------------------------------------------------------------- MODE/FEEL --
// Forced rather than left to EEPROM so the intended feel survives an EEPROM reset:
// full brightness (the cap in config.h is 200) and a long fade (`speed` 32 gives
// ~2 s to black; the default 127 is ~0.5 s).
void keyboard_post_init_user(void) {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_REACTIVE_SIMPLE);
    rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), 200);
    rgb_matrix_set_speed_noeeprom(32);
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    hsv_t accent_hsv = {HSV_BLUE};
    accent_hsv.v     = ACCENT_BRIGHTNESS;
    rgb_t accent     = hsv_to_rgb(accent_hsv);

    // A hit ages out of the effect after 65535/(speed+1) ms, so only leave the
    // accent off while the effect would still be drawing that LED.
    uint16_t max_tick = 65535 / ((uint16_t)rgb_matrix_get_speed() + 1);

    for (uint8_t i = 0; i < ARRAY_SIZE(led_accents); i++) {
        uint8_t led = led_accents[i];
        if (led < led_min || led >= led_max) {
            continue;
        }

        bool pulsing = false;
        for (int8_t j = g_last_hit_tracker.count - 1; j >= 0; j--) {
            if (g_last_hit_tracker.index[j] == led) {
                pulsing = g_last_hit_tracker.tick[j] < max_tick;
                break;
            }
        }
        if (!pulsing) {
            rgb_matrix_set_color(led, accent.r, accent.g, accent.b);
        }
    }

    hsv_t indicator_hsv = {HSV_BLUE};
    indicator_hsv.v     = INDICATOR_BRIGHTNESS;
    rgb_t indicator     = hsv_to_rgb(indicator_hsv);
    if (0 >= led_min && 0 < led_max) {
        rgb_matrix_set_color(0, indicator.r, indicator.g, indicator.b);
    }
    if (36 >= led_min && 36 < led_max) {
        rgb_matrix_set_color(36, indicator.r, indicator.g, indicator.b);
    }

    return false;
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
        // Returns true on purpose: keyboards/sofle/sofle.c's encoder_update_kb
        // then taps KC_VOLU/KC_VOLD once more, so one detent is two volume
        // steps. That is what this encoder has always done -- return false for
        // index 0 as well to halve it.
        return true;
    }

    // index == 1: per layer, and handled entirely here. Never fall through to
    // the keyboard's encoder_update_kb -- its index 1 branch taps KC_PGUP/
    // KC_PGDN unconditionally, which would page on top of every wheel or arrow
    // step below. Clockwise keeps the negative direction (KC_PGDN, MS_WHLD)
    // that the stock Sofle code uses.
    switch (get_highest_layer(layer_state | default_layer_state)) {
        case U_NAV:
            tap_code(clockwise ? KC_DOWN : KC_UP);
            break;
        case U_NUM:
        case U_SYM:
        case U_FUN:
        case U_MOUSE:
        case U_MEDIA:
        case U_BUTTON:
            tap_code(clockwise ? MS_WHLD : MS_WHLU);
            break;
        default: // U_BASE, U_EXTRA, U_TAP
            tap_code(clockwise ? KC_PGDN : KC_PGUP);
            break;
    }
    return false;
}

#endif
