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

// Currently recording a macro
static bool macro_record = false;

// Currently recording the mouse text
static bool mouse_record = false;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐┌───┬───┐
     * │Esc│ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ = │ Backsp││Ins│PgU│
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤├───┼───┤
     * │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │ [ │ ] │  \  ││Del│PgD│
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤└───┴───┘
     * │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │ ; │ ' │  Enter │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤┌───┐
     * │ Shift  │ Z │ X │ C │ V │ B │ N │ M │ , │ . │ / │ Shift    ││ ↑ │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┼───┴┬────┬─┬──┴┼───┼───┐
     * │Ctrl│GUI │Alt │                        │ Alt│ GUI│Ctrl│ │ ← │ ↓ │ → │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┘ └───┴───┴───┘
     */
    [MAC_LAYER] = LAYOUT(
        KC_ESC,                 KC_1,    KC_2,    KC_3,   KC_4,   KC_5,  KC_6, KC_7,   KC_8,  KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSLS, KC_GRV,  VIM_ON,
        LT(NAV_LAYER, KC_TAB),  KC_Q,    KC_W,    KC_E,   KC_R,   KC_T,  KC_Y, KC_U,   KC_I,  KC_O,    KC_P,    KC_LBRC, KC_RBRC,     KC_BSPC,      KC_PGUP,
        KC_LCTL,                KC_A,    KC_S,    KC_D,   KC_F,   KC_G,  KC_H, KC_J,   KC_K,  KC_L,    KC_SCLN, KC_QUOT, KC_NO,        KC_ENT,      KC_PGDN,
        KC_LSFT,                KC_NO,   KC_Z,    KC_X,   KC_C,   KC_V,  KC_B, KC_N,   KC_M,  KC_COMM, KC_DOT,  KC_SLSH,    KC_RSFT,    KC_UP,      MO(FN_LAYER),
        VIM_OFF,                KC_LALT, KC_LGUI,         KC_SPC,    TD(TD_VIM),    KC_SPC,           KC_RGUI,    KC_RCTL,    KC_LEFT,  KC_DOWN,    KC_RIGHT
    ),

    [WIN_LAYER] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,               
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_LGUI, KC_LALT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    [NAV_LAYER] = LAYOUT( // TAB hold
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_UP,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,               
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    [FN_LAYER] = LAYOUT(
        KC_TRNS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_TRNS, KC_TRNS, OSL(SAVE_LAYER),
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DM_PLY1, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DM_PLY2,               
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_VOLU, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                                     KC_MUTE, KC_VOLD, KC_MUTE
    ),

    [SAVE_LAYER] = LAYOUT( // OSL from FN Layer
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DM_REC1, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, UG_TOGG, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DM_REC2,               
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MD_RECORD_TOGGLE, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};

// const rgblight_segment_t PROGMEM layer_0_rgb[] = RGBLIGHT_LAYER_SEGMENTS({0, RGBLIGHT_LED_COUNT, 45, 255, 255});  // Sunflower
// const rgblight_segment_t PROGMEM layer_1_rgb[] = RGBLIGHT_LAYER_SEGMENTS({0, RGBLIGHT_LED_COUNT, 45, 255, 255});  // Sunflower
// const rgblight_segment_t PROGMEM layer_2_rgb[] = RGBLIGHT_LAYER_SEGMENTS({0, RGBLIGHT_LED_COUNT, 15, 180, 255});  // Coral
// const rgblight_segment_t PROGMEM layer_3_rgb[] = RGBLIGHT_LAYER_SEGMENTS({0, RGBLIGHT_LED_COUNT, 15, 180, 255});  // Coral
// const rgblight_segment_t PROGMEM layer_4_rgb[] = RGBLIGHT_LAYER_SEGMENTS({0, RGBLIGHT_LED_COUNT, 128, 255, 200}); // Teal
// const rgblight_segment_t PROGMEM layer_5_rgb[] = RGBLIGHT_LAYER_SEGMENTS({0, RGBLIGHT_LED_COUNT, 110, 255, 200}); // Emerald
// const rgblight_segment_t PROGMEM layer_6_rgb[] = RGBLIGHT_LAYER_SEGMENTS({0, RGBLIGHT_LED_COUNT, 215, 200, 255}); // Soft Purple
// const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
//     layer_0_rgb,
//     layer_1_rgb,
//     layer_2_rgb,
//     layer_3_rgb,
//     layer_4_rgb,
//     layer_5_rgb,
//     layer_6_rgb
// );

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

    // rgblight_layers = my_rgb_layers;
    rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING);
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

void set_rgb_from_layer(layer_state_t state) {
    // 1. Determine highest layer
    uint8_t current_layer = get_highest_layer(state);

    // 2. Clear all layer slots (0-5) to ensure no color mixing
    //for (uint8_t i = 0; i < 7; i++) {
    //    rgblight_set_layer_state(i, false);
    //}

    switch (current_layer) {
        case 0:
        case 1: {
            if (vim_mode_enabled()) {
                rgblight_sethsv_noeeprom(110, 255, 200);
                rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
            } else if (mouse_record || macro_record) {
                rgblight_sethsv_noeeprom(190, 150, 200);
                rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
            } else {
                rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING);
                rgblight_sethsv_noeeprom(45, 255, 255);
            }
            break;
        }
        case 2:
        case 3:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
            rgblight_sethsv_noeeprom(15, 180, 255);
            break;
        case 4:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
            rgblight_sethsv_noeeprom(128, 255, 200);
            break;
        case 5:
            rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
            rgblight_sethsv_noeeprom(110, 255, 200);
            break;
        default:
            break;
    }
}

bool dynamic_macro_record_start_user(int8_t direction) {
    macro_record = true;
    set_rgb_from_layer(layer_state);
    return true;
}
bool dynamic_macro_record_end_user(int8_t direction) {
    macro_record = false;
    set_rgb_from_layer(layer_state);
    return true;
}
void mousedraw_start_record_user() {
    set_rgb_from_layer(layer_state);
}
void mousedraw_stop_record_user() {
    set_rgb_from_layer(layer_state);
}
void vim_mode_changed_user(void) {
    set_rgb_from_layer(layer_state);
}

layer_state_t layer_state_set_user(layer_state_t state) {
	update_vim_from_state(state);
    set_rgb_from_layer(state); // This now handles everything including Vim
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