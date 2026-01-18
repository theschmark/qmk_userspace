#pragma once

#include QMK_KEYBOARD_H

void mousedraw_start_record(void);
void mousedraw_stop_record(void);
void mousedraw_play(bool mouse_held);

bool process_mousedraw(uint16_t keycode, keyrecord_t *record);