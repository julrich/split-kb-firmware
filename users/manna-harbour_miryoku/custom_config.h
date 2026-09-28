// Copyright 2019 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

// Germanisation of Miryoku's base layers. Two separate things:
//
// 1. The umlauts. On a German host layout the HID usages render as
//       0x33 (";:") = ö      0x34 ("'\"") = ä      0x2F ("[{") = ü
//    Miryoku's base has KC_QUOT on the home-row outer key, which therefore typed
//    "ä" on the key where a German keyboard has "ö". Substituting the layer (the
//    mechanism Miryoku documents in discussion #85) keeps the layout and moves
//    that cell to KC_SCLN = ö. The ä and ü keys themselves are the Sofle's
//    otherwise-unused outermost right-hand keys, mapped in the keymap's config.h.
//
// 2. **Why the alphas are QWERTY and not QWERTZ.**  HID usages are *positions*;
//    the host layout turns them into characters. A German host maps the US "y"
//    position (0x1C) to "z" and the US "z" position (0x1D) to "y" -- the host
//    does the Z/Y swap by itself. Miryoku's QWERTZ alphas move the usages to the
//    same positions a German keyboard has them, so on a German host the two
//    swaps cancel out and every key types its US legend (verified on 2026-09-28:
//    "y still prints y, z still prints z"). QWERTZ alphas is therefore the
//    setting for a *US* host; with a German host the alphas must stay QWERTY and
//    the physical result is German-correct: the key labelled "y" types "z".
//
// keymap_german.h is deliberately NOT included: this file is part of the
// config.h chain, which is also pulled into the assembly translation units, and
// that header drags in keycodes.h (the assembler chokes on it).
//
// EXTRA is Miryoku's alias for the base table (miryoku_layer_selection.h maps
// MIRYOKU_LAYER_EXTRA to MIRYOKU_ALTERNATIVES_BASE_QWERTY), so it follows along;
// TAP is a separate table (base alphas without the mod-taps) and gets the same
// one-cell change so the position stays consistent in every layer.
#if defined(MIRYOKU_ALPHAS_QWERTY)

#define MIRYOKU_LAYER_BASE \
KC_Q,              KC_W,              KC_E,              KC_R,              KC_T,              KC_Y,              KC_U,              KC_I,              KC_O,              KC_P,              \
LGUI_T(KC_A),      LALT_T(KC_S),      LCTL_T(KC_D),      LSFT_T(KC_F),      KC_G,              KC_H,              LSFT_T(KC_J),      LCTL_T(KC_K),      LALT_T(KC_L),      LGUI_T(KC_SCLN),   \
LT(U_BUTTON,KC_Z), ALGR_T(KC_X),      KC_C,              KC_V,              KC_B,              KC_N,              KC_M,              KC_COMM,           ALGR_T(KC_DOT),    LT(U_BUTTON,KC_SLSH),\
U_NP,              U_NP,              LT(U_MEDIA,KC_ESC),LT(U_NAV,KC_SPC),  LT(U_MOUSE,KC_TAB),LT(U_SYM,KC_ENT),  LT(U_NUM,KC_BSPC), LT(U_FUN,KC_DEL),  U_NP,              U_NP

#define MIRYOKU_LAYER_EXTRA MIRYOKU_LAYER_BASE

#define MIRYOKU_LAYER_TAP \
KC_Q,              KC_W,              KC_E,              KC_R,              KC_T,              KC_Y,              KC_U,              KC_I,              KC_O,              KC_P,              \
KC_A,              KC_S,              KC_D,              KC_F,              KC_G,              KC_H,              KC_J,              KC_K,              KC_L,              KC_SCLN,           \
KC_Z,              KC_X,              KC_C,              KC_V,              KC_B,              KC_N,              KC_M,              KC_COMM,           KC_DOT,            KC_SLSH,           \
U_NP,              U_NP,              KC_ESC,            KC_SPC,            KC_TAB,            KC_ENT,            KC_BSPC,           KC_DEL,            U_NP,              U_NP

#endif
