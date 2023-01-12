#include QMK_KEYBOARD_H

enum layer_names {
    /* _M_XYZ = Mac Os, _W_XYZ = Win/Linux */
    _QWERTY,
    _LOWER,
    _RAISE,
    
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/* QWERTY
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |  {[  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  |  ]}  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|  MUTE |    | HOME  |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *     | LCtrl| LGUI | LAlt |Enter |LOWER | / Space /       \ Bspc \  |RAISE | ENDS |  DEL | RESC | RCtrl|
 *     |      |      |      |      |      |/       /         \      \ |      |      |      |      |      |
 *      `-----------------------------------------'           '------''---------------------------------'
 */
[_QWERTY] = LAYOUT( \
  KC_GRV     , KC_1   , KC_2    , KC_3   , KC_4     , KC_5   ,                                            KC_6    , KC_7     , KC_8     , KC_9      , KC_0      , KC_MINUS    , \
  KC_LBRACKET, KC_Q   , KC_W    , KC_E   , KC_R     , KC_T   ,                                            KC_Y    , KC_U     , KC_I     , KC_O      , KC_P      , KC_RBRACKET , \
  KC_TAB     , KC_A   , KC_S    , KC_D   , KC_F     , KC_G   ,                                            KC_H    , KC_J     , KC_K     , KC_L      , KC_SCLN   , KC_QUOT     , \
  KC_LSFT    , KC_Z   , KC_X    , KC_C   , KC_V     , KC_B   , KC_MUTE  ,                     KC_HOME   , KC_N    , KC_M     , KC_COMM  , KC_DOT    , KC_SLSH   , KC_RSFT     , \
  KC_LCTRL   , KC_LGUI, KC_LALT , KC_ENT , XXXXXXX  , KC_SPC ,                                            KC_BSPC , XXXXXXX  , KC_END   , KC_DELETE , KC_ESCAPE , KC_RCTRL      \
  ),

[_LOWER] = LAYOUT( \
  XXXXXXX  , KC_F1       , KC_F2      , KC_F3      , KC_F4      , KC_F5       ,                                KC_F6          , KC_F7    , KC_F8    , KC_F9     , KC_F10     , KC_F11  , \
  XXXXXXX  , KC_KP_SLASH , KC_AMPR    , KC_ASTR    , KC_LPRN    , KC_RPRN     ,                                KC_KP_SLASH    , KC_7     , KC_8     , KC_9      , KC_0       , XXXXXXX , \
  KC_TAB   , KC_ASTR     , KC_DLR     , KC_PERC    , KC_CIRC    , KC_KP_MINUS ,                                KC_KP_ASTERISK , KC_4     , KC_5     , KC_6      , KC_RPRN    , XXXXXXX , \
  KC_LSFT  , XXXXXXX     , KC_EXLM    , KC_AT      , KC_HASH    , KC_KP_PLUS  , KC_MUTE ,            KC_HOME , KC_0           , KC_1     , KC_2     , KC_3      , KC_KP_PLUS , KC_LSFT , \
  KC_LCTRL , KC_LGUI     , KC_LALT    , KC_ENT     , XXXXXXX    , KC_SPC      ,                                KC_BSPC        , XXXXXXX  , KC_END   , KC_DELETE , KC_ESCAPE , KC_RCTRL  \
  ),

[_RAISE] = LAYOUT( \
  XXXXXXX  , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX  , XXXXXXX ,                                XXXXXXX  , XXXXXXX  ,  XXXXXXX , XXXXXXX   , XXXXXXX   , XXXXXXX , \
  XXXXXXX  , KC_INS  , KC_PSCR , KC_APP  , XXXXXXX  , XXXXXXX ,                                XXXXXXX  , KC_UP    , KC_MNXT  , XXXXXXX   , KC_BSPC   , XXXXXXX , \
  KC_TAB   , KC_LALT , KC_LCTL , KC_LSFT , XXXXXXX  , KC_CAPS ,                                KC_LEFT  , KC_DOWN  , KC_RGHT  , KC_DEL    , KC_BSPC   , XXXXXXX , \
  KC_LSFT  , KC_UNDO , KC_CUT  , KC_COPY , KC_PASTE , XXXXXXX , KC_MUTE ,            KC_HOME , XXXXXXX  , XXXXXXX  , XXXXXXX  , XXXXXXX   , XXXXXXX   , XXXXXXX , \
  KC_LCTRL , KC_LGUI , KC_LALT , KC_ENT  , XXXXXXX  , KC_SPC  ,                                KC_BSPC  , XXXXXXX    , KC_END   , KC_DELETE , KC_ESCAPE , KC_RCTRL  \
  ),

};

// #ifdef ENCODER_ENABLE

// bool encoder_update_user(uint8_t index, bool clockwise) {
//     if (index == 0) {
//         if (clockwise) {
//             tap_code(KC_VOLU);
//         } else {
//             tap_code(KC_VOLD);
//         }
//     } else if (index == 1) {
//         if (clockwise) {
//             tap_code(KC_WH_D);
//         } else {
//             tap_code(KC_WH_U);
//         }
//     }
//     return true;
// }

// #endif
