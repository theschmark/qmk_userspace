#include QMK_KEYBOARD_H
#include "qmk-vim/src/vim.h"

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
};

// Override shift+backspace to be delete
const key_override_t delete_key_override = ko_make_basic(MOD_MASK_SHIFT, KC_BSPC, KC_DEL);
const key_override_t* key_overrides[] = {
	&delete_key_override
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

	[MAC_LAYER] = LAYOUT_all(
		KC_ESC,                 KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSLS, KC_GRV,  VIM_ON,
		LT(NAV_LAYER, KC_TAB),  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSPC,          KC_PGUP,
		KC_LCTL,                KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,                    KC_PGDN,
		KC_LSFT,                KC_NO,   KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   MO(FN_LAYER),
		VIM_OFF,                KC_LALT, KC_LGUI,                            KC_SPC,                             KC_RGUI, KC_RCTL,          KC_LEFT, KC_DOWN, KC_RGHT),

	[WIN_LAYER] = LAYOUT_all(
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                   KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS, KC_TRNS,
		KC_TRNS, KC_LGUI, KC_LALT,                            KC_TRNS,                            KC_TRNS, KC_TRNS,          KC_TRNS, KC_TRNS, KC_TRNS),

	[NAV_LAYER] = LAYOUT_all( // TAB hold
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_TRNS, KC_TRNS, KC_TRNS,                   KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS,                            VIM_ON,                             KC_TRNS, KC_TRNS,          KC_TRNS, KC_TRNS, KC_TRNS),


	[FN_LAYER] = LAYOUT_all(
		KC_TRNS,  KC_F1,  KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_TRNS, KC_TRNS, MO(SAVE_LAYER),
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          DM_PLY1,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                   DM_PLY2,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_VOLU, KC_TRNS,
		KC_TRNS, TO(WIN_LAYER), TO(MAC_LAYER),             KC_TRNS,                            KC_TRNS, KC_TRNS,             KC_MUTE, KC_VOLD, KC_MUTE),

    [SAVE_LAYER] = LAYOUT_all( // OSL from FN Layer
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          DM_REC1,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                   DM_REC2,
		KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS, KC_TRNS,
		KC_TRNS, PDF(WIN_LAYER), PDF(MAC_LAYER),              KC_TRNS,                            KC_TRNS, KC_TRNS,          KC_TRNS, KC_TRNS, KC_TRNS),
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
	// debug_enable=true;

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

	switch(keycode) {
		case VIM_ON:
			enable_vim_mode();
			return false;

		case VIM_OFF:
			disable_vim_mode();
			return false;

		default:
			return true;
	}
}