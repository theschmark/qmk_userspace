// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "debug.h"
#include "qmk-vim/src/vim.h"
#include "mousedraw/mousedraw.h"

enum layers {
	MAC_LAYER = 0, // Default for mac connection, FN + Super to enable
	WIN_LAYER,  // Default for windows connection, FN + Alt to enable
	NAV_LAYER,  // Common navigation
	FN_LAYER, // FN
	SAVE_LAYER,  // OSL from FN + Top Right
};

enum custom_keycodes {
	VIM_ON = QK_KB_0,
	VIM_OFF,
    MD_RECORD_TOGGLE,
	MD_PLAY,
};

// Override shift+backspace to be delete
const key_override_t delete_key_override = ko_make_basic(MOD_MASK_SHIFT, KC_BSPC, KC_DEL);
const key_override_t* key_overrides[] = {
	&delete_key_override
};

enum tapdance_keycodes {
    TD_VIM,
};

typedef struct {
    bool is_press_action;
    int state;
} tap;

// Define the states
enum {
    SINGLE_TAP = 1,
    SINGLE_HOLD = 2,
    DOUBLE_TAP = 3,
};

// TD state management
static tap td_vim_dance_state;

// Currently recording the mouse text
static bool mouse_record = false;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* 
     * ,---------------------------------------------------------------------------------------------------------------.
     * | ESC  |  1   |  2   |  3   |  4   |  5   |  6   |  7   |  8   |  9   |  0   |  -   |  =   |  \   |  `   | VIMON|
     * |------+------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * | TAB(NAV) |  Q   |  W   |  E   |  R   |  T   |  Y   |  U   |  I   |  O   |  P   |  [   |  ]   |  BKSP   | PGUP |
     * |----------+------+------+------+------+------+------+------+------+------+------+------+------+---------+------|
     * | LCTL       |  A   |  S   |  D   |  F   |  G   |  H   |  J   |  K   |  L   |  ;   |  '   |    ENTER     | PGDN |
     * |------------+------+------+------+------+------+------+------+------+------+------+------+--------------+------|
     * | LSFT          |  Z   |  X   |  C   |  V   |  B   |  N   |  M   |  ,   |  .   |  /   | RSFT      |  UP  | FN   |
     * |---------------+------+------+------+------+------+------+------+------+------+------+-----------+------+------|
     * | VIMOFF | LALT   | LGUI   |      SPACE     |  VIM |     SPACE        | RGUI   | RCTL   |  | LEFT | DOWN | RIGHT|
     * `----------------------------------------------------------------------------------------  ---------------------'
     */
    [MAC_LAYER] = LAYOUT(
        KC_ESC,                 KC_1,    KC_2,    KC_3,   KC_4,   KC_5,  KC_6, KC_7,   KC_8,  KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSLS, KC_GRV,  VIM_ON,
        LT(NAV_LAYER, KC_TAB),  KC_Q,    KC_W,    KC_E,   KC_R,   KC_T,  KC_Y, KC_U,   KC_I,  KC_O,    KC_P,    KC_LBRC, KC_RBRC,     KC_BSPC,      KC_PGUP,
        KC_LCTL,                KC_A,    KC_S,    KC_D,   KC_F,   KC_G,  KC_H, KC_J,   KC_K,  KC_L,    KC_SCLN, KC_QUOT, KC_ENT,      KC_PGDN,
        KC_LSFT,                KC_Z,    KC_X,    KC_C,   KC_V,   KC_B,  KC_N, KC_M,   KC_COMM, KC_DOT,  KC_SLSH,    KC_RSFT,    KC_UP,      MO(FN_LAYER),
        VIM_OFF,                KC_LALT, KC_LGUI,         KC_SPC,    TD(TD_VIM),    KC_SPC,           KC_RGUI,    KC_RCTL,    KC_LEFT,  KC_DOWN,    KC_RIGHT
    ),

    /*
     * ,---------------------------------------------------------------------------------------------------------------.
     * |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |
     * |------+------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |      --     |  --  |
     * |      |------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * |  --      |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |       --       |  --  |
     * |----------+------+------+------+------+------+------+------+------+------+------+------+----------------+------|
     * |  --           |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --       |  --  |  --  |
     * |---------------+------+------+------+------+------+------+------+------+------+------+-----------+------+------|
     * |  --   | LGUI   | LALT   |       --       |  --  |       --          |  --    |  --    |  |  --  |  --  |  --  |
     * `---------------------------------------------------------------------------------------------------------------'
     */
    [WIN_LAYER] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,               
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_LGUI, KC_LALT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    /*
     * ,---------------------------------------------------------------------------------------------------------------.
     * |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |
     * |------+------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * |  --     |  --  |  UP  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |    --    |  --  |
     * |------|------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * |  --      | LEFT | DOWN | RGHT |  --  |  --  | LEFT | DOWN |  UP  | RGHT |  --  |  --  |     --         |  --  |
     * |----------+------+------+------+------+------+------+------+------+------+------+------+----------------+------|
     * |  --           |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --       |  --  |  --  |
     * |---------------+------+------+------+------+------+------+------+------+------+------+-----------+------+------|
     * |  --   |  --    |  --    |      --        |  --  |       --          |  --    |  --    |  |  --  |  --  |  --  |
     * `---------------------------------------------------------------------------------------------------------------'
     */ 
    [NAV_LAYER] = LAYOUT( // TAB hold
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_UP,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,               
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    /*
     * ,---------------------------------------------------------------------------------------------------------------.
     * |  --  |  F1  |  F2  |  F3  |  F4  |  F5  |  F6  |  F7  |  F8  |  F9  |  F10 |  F11 |  F12 |  -- | -- | OSL(SV) |
     * |------+------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |      --     | PLY1 |
     * |------|------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * |  --      |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |      --        | PLY2 |
     * |----------+------+------+------+------+------+------+------+------+------+------+------+----------------+------|
     * |  --           |  --  |  --  |  --  |  --  |  --  |  --  | MD_PLAY |  --  |  --  |  --  |  --    | VOLU |  --  |
     * |---------------+------+------+------+------+------+------+------+------+------+------+-----------+------+------|
     * |  --   |  --    |  --    |      --        |  --  |       --          |  --    |  --    |  | MUTE | VOLD | MUTE |
     * `---------------------------------------------------------------------------------------------------------------'
     */
    [FN_LAYER] = LAYOUT(
        KC_TRNS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_TRNS, KC_TRNS, OSL(SAVE_LAYER),
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DM_PLY1, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DM_PLY2,               
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MD_PLAY, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_VOLU, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                                     KC_MUTE, KC_VOLD, KC_MUTE
    ),

    /*
     * ,---------------------------------------------------------------------------------------------------------------.
     * |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |
     * |------+------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |      --     | REC1 |
     * |------|------+------+------+------+------+------+------+------+------+------+------+------+-------------+------|
     * |  --      |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |  --  |      --        | REC2 |
     * |----------+------+------+------+------+------+------+------+------+------+------+------+----------------+------|
     * |  --           |  --  |  --  |  --  |  --  |  --  |  --  | MD_REC  |  --  |  --  |  --  |  --    |  --  |  --  |
     * |---------------+------+------+------+------+------+------+------+------+------+------+-----------+------+------|
     * |  --   |  --    |  --    |      --        |  --  |       --          |  --    |  --    |  |  --  |  --  |  --  |
     * `---------------------------------------------------------------------------------------------------------------'
     */
    */
    [SAVE_LAYER] = LAYOUT( // OSL from FN Layer
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DM_REC1, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DM_REC2,               
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MD_RECORD_TOGGLE, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};

int current_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        if (!state->pressed) return SINGLE_TAP;
        // Note that state->interrupted is true if I hit a key before 'held' is recognized by the timeout
        return SINGLE_HOLD;
    } else if (state->count == 2) {
        return DOUBLE_TAP;
    } else return 8; // Magic number for "too many taps"
}

void td_vim_finished(tap_dance_state_t *state, void *user_data) {
    td_vim_dance_state.state = current_dance(state);
    switch (td_vim_dance_state.state) {
        case SINGLE_HOLD:
            layer_on(NAV_LAYER); 
            break;
        case DOUBLE_TAP:
            toggle_vim_mode();
            break;
    }
}

void td_vim_reset(tap_dance_state_t *state, void *user_data) {
    switch (td_vim_dance_state.state) {
        case SINGLE_HOLD:
            layer_off(NAV_LAYER);
            break;
        case DOUBLE_TAP:
            // Ignore, no need to disable vim
            break;
    }
    td_vim_dance_state.state = 0;
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_VIM] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_vim_finished, td_vim_reset),
};

void update_vim_from_state(layer_state_t state) {
	if (IS_LAYER_ON_STATE(state, WIN_LAYER)) {
		disable_vim_for_mac();
	} else {
		enable_vim_for_mac();
	}
}

// Fix the default layer state bsed on the default_layer_state from eeprom at startup
void keyboard_post_init_user(void) {
	//debug_enable=true;

    layer_state_set(default_layer_state);

	// Called as it's not clear if layer_state_set_user is called from the above
	update_vim_from_state(default_layer_state);
}

// Override based on the OS detection
bool process_detected_host_os_user(os_variant_t detected_os) {
	 switch (detected_os) {
        case OS_MACOS:
        case OS_IOS:
			layer_move(MAC_LAYER);
            break;
        case OS_WINDOWS:
        case OS_LINUX:
			layer_move(WIN_LAYER);
            break;
        case OS_UNSURE:
			// Do nothing, keep the default detected based on layers.
            break;
    }
    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
	update_vim_from_state(state);
    return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Process case modes
    if (!process_vim_mode(keycode, record)) {
        return false;
    }

	if (!process_mousedraw(keycode, record)) {
		return false;
	}

    if (record->event.pressed) {
		switch(keycode) {
			case VIM_ON:
				enable_vim_mode();
				return false;

			case VIM_OFF:
				disable_vim_mode();
				return false;

			case MD_RECORD_TOGGLE:
				if (mouse_record) {
					mousedraw_stop_record();
				} else {
					mousedraw_start_record();
				}
				mouse_record = !mouse_record;
				return false;

			case MD_PLAY:
				mousedraw_play(false);
				return false;

			default:
				break;
		}
	}
	return true;
}