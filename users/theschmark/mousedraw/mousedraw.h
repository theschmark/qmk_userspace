#pragma once

#include QMK_KEYBOARD_H

void mousedraw_start_record_user(void);
void mousedraw_stop_record_user(void);

void mousedraw_start_record(void);
void mousedraw_stop_record(void);
void mousedraw_play(bool mouse_held);

void mousedraw_inc_font(void);
void mousedraw_dec_font(void);

void mousedraw_inc_speed(void);
void mousedraw_dec_speed(void);

void mousedraw_inc_interval(void);
void mousedraw_dec_interval(void);

bool process_mousedraw(uint16_t keycode, keyrecord_t *record);