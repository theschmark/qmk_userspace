#include QMK_KEYBOARD_H
#include <math.h>
#include "mousedraw.h"
#include "deferred_exec.h"

// --- Configuration ---
#define BUFFER_MAX     64     

static int report_interval = 8;
static float font_size = 90.0f;
static float draw_speed = 4.0f;

void mousedraw_inc_font() {
    font_size += 10.0f;
}
void mousedraw_dec_font() {
    font_size -= 10.0f;
}
void mousedraw_inc_speed() {
    draw_speed += 0.25f;
}
void mousedraw_dec_speed() {
    draw_speed -= 0.25f;
}
void mousedraw_inc_interval() {
    report_interval += 2;
}
void mousedraw_dec_interval() {
    report_interval -= 2;
}

// --- Structures ---
typedef struct { float x, y; } Point;
typedef enum { MOVE, LINE, CURVE } SegmentType;

typedef struct {
    SegmentType type;
    Point p1, p2, dest;
} Segment;

typedef struct {
    const Segment* segments;
    int segment_count;
    float width;
} Glyph;

// Recording
static const Glyph* record_buffer[BUFFER_MAX];
static int record_len = 0;

// State machine state
static bool is_recording = false, is_drawing = false;


// Drawing state
static int cur_char=0, cur_seg=0;
static float t_prog=0, word_x_off=0, sub_x=0, sub_y=0;
static Point last_abs={0,0}, seg_start={0,0};

// Forward declares
uint32_t drawing_callback(uint32_t trigger_time, void *cb_arg);

// API
void mousedraw_start_record(void) {
    record_len = 0;
    is_drawing = false;
    is_recording = true;
}

void mousedraw_stop_record(void) {
    is_recording = false;
}

void mousedraw_play(bool mouse_held) {
    if (!is_recording && record_len > 0) {
        if (is_drawing) {
            is_drawing = false;
        } else {
            is_drawing = true; cur_char = 0; cur_seg = 0; t_prog = 0;
            word_x_off = 0; sub_x = 0; sub_y = 0;
            last_abs = (Point){0,0.0f}; seg_start = (Point){0,-1.0f};
            defer_exec(report_interval, drawing_callback, NULL);
        }
    }
}

// --- Glyph Library ---
// Normalized coordinates: y=0.0 (top), y=1.0 (baseline), y=1.3 (descender)
// Every letter now explicitly ends with a MOVE to {width, 1.0} 
// to 'zero out' the vertical drift before the next char starts.

static const Segment char_a[] = {{MOVE,{0,0},{0,0},{0.8,0.5}}, {CURVE,{0.8,0.1},{0.1,0.1},{0.1,0.6}}, {CURVE,{0.1,1.0},{0.8,1.0},{0.8,0.5}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_b[] = {{MOVE,{0,0},{0,0},{0.2,0.0}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.2,0.7}}, {CURVE,{0.2,0.4},{0.8,0.4},{0.8,0.7}}, {CURVE,{0.8,1.0},{0.2,1.0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_c[] = {{MOVE,{0,0},{0,0},{0.8,0.5}}, {CURVE,{0.8,0.3},{0.2,0.3},{0.2,0.6}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_d[] = {{MOVE,{0,0},{0,0},{0.8,0.5}}, {CURVE,{0.8,0.1},{0.2,0.1},{0.2,0.5}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.5}}, {MOVE,{0,0},{0,0},{0.8,0.0}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_e[] = {{MOVE,{0,0},{0,0},{0.2,0.7}}, {LINE,{0,0},{0,0},{0.8,0.7}}, {CURVE,{0.8,0.3},{0.2,0.3},{0.2,0.6}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,1.0}}};
static const Segment char_f[] = {{MOVE,{0,0},{0,0},{0.7,0.1}}, {CURVE,{0.5,0.0},{0.3,0.0},{0.3,0.2}}, {LINE,{0,0},{0,0},{0.3,1.0}}, {MOVE,{0,0},{0,0},{0.1,0.4}}, {LINE,{0,0},{0,0},{0.5,0.4}}, {MOVE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_g[] = {{MOVE,{0,0},{0,0},{0.8,0.5}}, {CURVE,{0.8,0.1},{0.2,0.1},{0.2,0.5}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.5}}, {LINE,{0,0},{0,0},{0.8,1.3}}, {CURVE,{0.8,1.5},{0.1,1.5},{0.1,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_h[] = {{MOVE,{0,0},{0,0},{0.2,0.0}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.2,0.6}}, {CURVE,{0.2,0.3},{0.8,0.3},{0.8,0.6}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_i[] = {{MOVE,{0,0},{0,0},{0.5,0.4}}, {LINE,{0,0},{0,0},{0.5,1.0}}, {MOVE,{0,0},{0,0},{0.5,0.1}}, {LINE,{0,0},{0,0},{0.5,0.15}}, {MOVE,{0,0},{0,0},{0.5,1.0}}};
static const Segment char_j[] = {{MOVE,{0,0},{0,0},{0.6,0.4}}, {LINE,{0,0},{0,0},{0.6,1.3}}, {CURVE,{0.6,1.5},{0.1,1.5},{0.1,1.0}}, {MOVE,{0,0},{0,0},{0.6,0.1}}, {LINE,{0,0},{0,0},{0.6,0.15}}, {MOVE,{0,0},{0,0},{0.6,1.0}}};
static const Segment char_k[] = {{MOVE,{0,0},{0,0},{0.2,0.0}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.7,0.4}}, {LINE,{0,0},{0,0},{0.2,0.7}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_l[] = {{MOVE,{0,0},{0,0},{0.4,0.0}}, {LINE,{0,0},{0,0},{0.4,1.0}}, {MOVE,{0,0},{0,0},{0.5,1.0}}};
static const Segment char_m[] = {{MOVE,{0,0},{0,0},{0.1,1.0}}, {LINE,{0,0},{0,0},{0.1,0.4}}, {CURVE,{0.1,0.2},{0.45,0.2},{0.45,0.5}}, {LINE,{0,0},{0,0},{0.45,1.0}}, {MOVE,{0,0},{0,0},{0.45,0.5}}, {CURVE,{0.45,0.2},{0.8,0.2},{0.8,0.5}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{1.3,1.0}}};
static const Segment char_n[] = {{MOVE,{0,0},{0,0},{0.2,1.0}}, {LINE,{0,0},{0,0},{0.2,0.4}}, {CURVE,{0.2,0.2},{0.8,0.2},{0.8,0.5}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_o[] = {{MOVE,{0,0},{0,0},{0.5,0.3}}, {CURVE,{0.1,0.3},{0.1,1.0},{0.5,1.0}}, {CURVE,{0.9,1.0},{0.9,0.3},{0.5,0.3}}, {MOVE,{0,0},{0,0},{1.0,1.0}}};
static const Segment char_p[] = {{MOVE,{0,0},{0,0},{0.2,1.3}}, {LINE,{0,0},{0,0},{0.2,0.4}}, {CURVE,{0.2,0.2},{0.8,0.2},{0.8,0.5}}, {CURVE,{0.8,0.9},{0.2,0.9},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_q[] = {{MOVE,{0,0},{0,0},{0.8,0.5}}, {CURVE,{0.8,0.2},{0.2,0.2},{0.2,0.5}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.5}}, {LINE,{0,0},{0,0},{0.8,1.3}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_r[] = {{MOVE,{0,0},{0,0},{0.2,1.0}}, {LINE,{0,0},{0,0},{0.2,0.4}}, {MOVE,{0,0},{0,0},{0.2,0.6}}, {CURVE,{0.2,0.3},{0.6,0.3},{0.8,0.5}}, {MOVE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_s[] = {{MOVE,{0,0},{0,0},{0.8,0.4}}, {CURVE,{0.1,0.3},{0.1,0.6},{0.5,0.6}}, {CURVE,{0.9,0.6},{0.9,1.0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_t[] = {{MOVE,{0,0},{0,0},{0.4,0.1}}, {LINE,{0,0},{0,0},{0.4,0.9}}, {CURVE,{0.4,1.0},{0.6,1.0},{0.7,1.0}}, {MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.6,0.4}}, {MOVE,{0,0},{0,0},{0.7,1.0}}};
static const Segment char_u[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.2,0.8}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.8}}, {LINE,{0,0},{0,0},{0.8,0.4}}, {MOVE,{0,0},{0,0},{0.8,0.8}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_v[] = {{MOVE,{0,0},{0,0},{0.1,0.4}}, {LINE,{0,0},{0,0},{0.5,1.0}}, {LINE,{0,0},{0,0},{0.9,0.4}}, {MOVE,{0,0},{0,0},{1.0,1.0}}};
static const Segment char_w[] = {{MOVE,{0,0},{0,0},{0.1,0.4}}, {LINE,{0,0},{0,0},{0.3,1.0}}, {LINE,{0,0},{0,0},{0.5,0.6}}, {LINE,{0,0},{0,0},{0.7,1.0}}, {LINE,{0,0},{0,0},{0.9,0.4}}, {MOVE,{0,0},{0,0},{1.3,1.0}}};
static const Segment char_x[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.8,0.4}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_y[] = {{MOVE,{0,0},{0,0},{0.1,0.4}}, {LINE,{0,0},{0,0},{0.5,1.0}}, {LINE,{0,0},{0,0},{0.9,0.4}}, {MOVE,{0,0},{0,0},{0.5,1.0}}, {LINE,{0,0},{0,0},{0.2,1.5}}, {MOVE,{0,0},{0,0},{0.9,1.0}}};
static const Segment char_z[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.8,0.4}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {LINE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_sp[] = {};

static const Glyph glyph_a={char_a,4,0.9f},  glyph_b={char_b,5,0.9f},  glyph_c={char_c,4,0.8f},  glyph_d={char_d,5,0.9f},
            glyph_e={char_e,4,0.8f},  glyph_f={char_f,6,0.8f},  glyph_g={char_g,5,0.9f},  glyph_h={char_h,5,0.8f},
            glyph_i={char_i,5,0.5f},  glyph_j={char_j,6,0.5f},  glyph_k={char_k,5,0.8f},  glyph_l={char_l,2,0.5f},
            glyph_m={char_m,7,1.3f},  glyph_n={char_n,4,0.9f},  glyph_o={char_o,4,0.9f},  glyph_p={char_p,4,0.9f},
            glyph_q={char_q,5,0.9f},  glyph_r={char_r,5,0.8f},  glyph_s={char_s,3,0.8f},  glyph_t={char_t,6,0.5f},
            glyph_u={char_u,6,0.9f},  glyph_v={char_v,4,0.9f},  glyph_w={char_w,6,1.3f},  glyph_x={char_x,5,0.9f},
            glyph_y={char_y,6,0.9f},  glyph_z={char_z,4,0.8f},  glyph_space={char_sp,0,0.6f};

const Glyph* char_to_glyph(char c) {
    if (c == ' ') return &glyph_space;
    if (c >= 'a' && c <= 'z') {
        const Glyph* table[] = {&glyph_a,&glyph_b,&glyph_c,&glyph_d,&glyph_e,&glyph_f,&glyph_g,&glyph_h,&glyph_i,&glyph_j,&glyph_k,&glyph_l,&glyph_m,&glyph_n,&glyph_o,&glyph_p,&glyph_q,&glyph_r,&glyph_s,&glyph_t,&glyph_u,&glyph_v,&glyph_w,&glyph_x,&glyph_y,&glyph_z};
        return table[c - 'a'];
    }
    return NULL;
}

Point get_point(Segment s, Point start, float t) {
    Point p;
    if (s.type == CURVE) {
        float it = 1.0f - t;
        p.x = it*it*it*start.x + 3*it*it*t*s.p1.x + 3*it*t*t*s.p2.x + t*t*t*s.dest.x;
        p.y = it*it*it*start.y + 3*it*it*t*s.p1.y + 3*it*t*t*s.p2.y + t*t*t*s.dest.y;
    } else {
        p.x = start.x + t * (s.dest.x - start.x);
        p.y = start.y + t * (s.dest.y - start.y);
    }
    return p;
}

// Replace your t_prog increment with this logic
float segment_length(Segment s, Point start) {
    if (s.type == LINE || s.type == MOVE) {
        return sqrtf(powf(s.dest.x - start.x, 2) + powf(s.dest.y - start.y, 2));
    }
    // Approximation for Bezier length
    return sqrtf(powf(s.dest.x - start.x, 2) + powf(s.dest.y - start.y, 2)) * 1.2f;
}

// Add this to your state variables
static bool is_waiting = false;
static int wait_ticks = 0;

uint32_t drawing_callback(uint32_t trigger_time, void *cb_arg) {
    if (!is_drawing) {
        host_mouse_send(&(report_mouse_t){.buttons = 0}); 
        return 0;
    }

    // --- NEW: Dwell Time to clear OS acceleration ---
    if (is_waiting) {
        if (++wait_ticks > 3) { // Wait ~24ms between letters
            is_waiting = false;
            wait_ticks = 0;
        }
        return report_interval;
    }
    
    const Glyph* g = record_buffer[cur_char];

    if (g->segment_count == 0) { // Space handling
        word_x_off += g->width;
        seg_start.x = (last_abs.x / font_size) - word_x_off;
        seg_start.y = (last_abs.y / font_size);
        host_mouse_send(&(report_mouse_t){.buttons = 0});
        if (++cur_char >= record_len) { is_drawing = false; return report_interval; }
        is_waiting = true; // Clear buffer after space
        return report_interval;
    }

    Segment s = g->segments[cur_seg];
    Point p = get_point(s, seg_start, t_prog);
    
    float wx = (p.x + word_x_off) * font_size;
    float wy = p.y * font_size;

    float dx = (wx - last_abs.x) + sub_x;
    float dy = (wy - last_abs.y) + sub_y;
    int8_t rx = (int8_t)roundf(dx);
    int8_t ry = (int8_t)roundf(dy);
    
    sub_x = dx - rx; 
    sub_y = dy - ry;

    report_mouse_t report = {.x = rx, .y = ry, .buttons = (s.type == MOVE ? 0 : MOUSE_BTN1)};
    host_mouse_send(&report);

    last_abs = (Point){wx, wy};

    // Inside the callback:
    float seg_len = segment_length(s, seg_start);
    // Instead of a fixed increment, scale by length so speed is constant px/report
    float velocity_step = (draw_speed/ (seg_len * font_size)); 
    t_prog += velocity_step;
    // t_prog += draw_speed;

    if (t_prog >= 1.0f) {
        t_prog = 0; 
        seg_start = s.dest; 

        if (++cur_seg >= g->segment_count) {
            word_x_off += g->width; 
            cur_seg = 0; 
            seg_start.x = (last_abs.x / font_size) - word_x_off;
            seg_start.y = (last_abs.y / font_size);

            if (++cur_char >= record_len) { is_drawing = false; }
            else { is_waiting = true; } // Trigger dwell before next letter
        }
    }
    return report_interval;
}

bool process_mousedraw(uint16_t keycode, keyrecord_t *record) {
    // Silently skip anything else :)
    if (record->event.pressed &&
        is_recording &&
        record_len < BUFFER_MAX) {
                
        char c = (keycode >= KC_A && keycode <= KC_Z) ? (keycode - KC_A + 'a') : (keycode == KC_SPC ? ' ' : 0);
        if (c) {
            const Glyph* g = char_to_glyph(c);
            if (g) record_buffer[record_len++] = g;
            return false;
        }
    }
    return true;
}