// Copyright 2019 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

// German build (the keymap's rules.mk sets MIRYOKU_ALPHAS = QWERTZ).
//
// Miryoku's QWERTZ base has KC_QUOT ("'\"") on the home-row outer key. On a
// German host layout that usage renders as "ä", so the key next to L -- where a
// German keyboard has "ö" -- typed "ä". Substituting the layer keeps Miryoku's
// layout but moves the cell to KC_SCLN, which is "ö" on a German layout. This is
// Miryoku's own documented layer-substitution mechanism (discussion #85).
//
//   DE_ODIA = KC_SCLN = ö
//
// keymap_german.h is deliberately NOT included: custom_config.h is part of the
// config.h chain, which is also pulled into the assembly translation units, and
// that header drags in keycodes.h.
//
// EXTRA is Miryoku's alias for the base table (miryoku_layer_selection.h maps
// MIRYOKU_LAYER_EXTRA to MIRYOKU_ALTERNATIVES_BASE_QWERTZ), so it follows along;
// TAP is a separate table (base alphas without the mod-taps) and gets the same
// one-cell change so the position stays consistent in every layer.
//
// The two umlaut keys proper (ä, ü) are on the Sofle's unused outermost column,
// mapped in the keymap's config.h.
#if defined(MIRYOKU_ALPHAS_QWERTZ)

#define MIRYOKU_LAYER_BASE \
KC_Q,              KC_W,              KC_E,              KC_R,              KC_T,              KC_Z,              KC_U,              KC_I,              KC_O,              KC_P,              \
LGUI_T(KC_A),      LALT_T(KC_S),      LCTL_T(KC_D),      LSFT_T(KC_F),      KC_G,              KC_H,              LSFT_T(KC_J),      LCTL_T(KC_K),      LALT_T(KC_L),      LGUI_T(KC_SCLN),   \
LT(U_BUTTON,KC_Y), ALGR_T(KC_X),      KC_C,              KC_V,              KC_B,              KC_N,              KC_M,              KC_COMM,           ALGR_T(KC_DOT),    LT(U_BUTTON,KC_SLSH),\
U_NP,              U_NP,              LT(U_MEDIA,KC_ESC),LT(U_NAV,KC_SPC),  LT(U_MOUSE,KC_TAB),LT(U_SYM,KC_ENT),  LT(U_NUM,KC_BSPC), LT(U_FUN,KC_DEL),  U_NP,              U_NP

#define MIRYOKU_LAYER_EXTRA MIRYOKU_LAYER_BASE

#define MIRYOKU_LAYER_TAP \
KC_Q,              KC_W,              KC_E,              KC_R,              KC_T,              KC_Z,              KC_U,              KC_I,              KC_O,              KC_P,              \
KC_A,              KC_S,              KC_D,              KC_F,              KC_G,              KC_H,              KC_J,              KC_K,              KC_L,              KC_SCLN,           \
KC_Y,              KC_X,              KC_C,              KC_V,              KC_B,              KC_N,              KC_M,              KC_COMM,           KC_DOT,            KC_SLSH,           \
U_NP,              U_NP,              KC_ESC,            KC_SPC,            KC_TAB,            KC_ENT,            KC_BSPC,           KC_DEL,            U_NP,              U_NP

#endif
