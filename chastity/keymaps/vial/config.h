#pragma once

/* USB Device descriptor parameter */
#define VENDOR_ID       0xFC32
#define PRODUCT_ID      0x0287
#define DEVICE_VER      0x0001
#define MANUFACTURER    streetRAT
#define PRODUCT         Chastity

/* key matrix size */
// Rows are doubled-up
#define MATRIX_ROWS 10
#define MATRIX_COLS  7

// wiring of each half
#define MATRIX_ROW_PINS { D4, C6, D7, E6, B4 }
#define MATRIX_COL_PINS { B6, B2, B3, B1, F7, F6, B5 }
#define DIODE_DIRECTION COL2ROW

#define TAPPING_TERM 100
#define DEBOUNCE 5

/* encoder support */
#define ENCODERS_PAD_A { F5 }
#define ENCODERS_PAD_B { F4 }
#define ENCODERS_PAD_A_RIGHT { F4 }
#define ENCODERS_PAD_B_RIGHT { F5 }
#define ENCODER_RESOLUTION 2

#define TAP_CODE_DELAY 10

/* communication between sides */
#define USE_SERIAL
#define SERIAL_USE_MULTI_TRANSACTION
#define SOFT_SERIAL_PIN D2
#define MASTER_LEFT

#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION

#define VIAL_COMBO_ENTRIES 4

#define VIAL_KEYBOARD_UID {0x2A, 0x05, 0x99, 0xBB, 0xA3, 0xA4, 0x44, 0xA8}

#define SPLIT_USB_DETECT
