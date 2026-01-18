#include QMK_KEYBOARD_H
#include <math.h>
#include "mousedraw.h"
#include "deferred_exec.h"

// --- Configuration ---
#define FONT_SIZE      90.0f  
#define DRAW_SPEED     0.04f  
#define REPORT_INTERVAL 10    
#define BUFFER_MAX     64     

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
        is_drawing = true;
        cur_char = 0;
        cur_seg = 0;
        t_prog = 0;
        word_x_off = 0;
        sub_x = 0;
        sub_y = 0;
        last_abs = (Point){0,0}; seg_start = (Point){0,0};
        defer_exec(REPORT_INTERVAL, drawing_callback, NULL);
    }
}

// --- Glyph Library ---
static const Segment char_a[] = {{MOVE,{0,0},{0,0},{0.8,0.4}}, {CURVE,{0.8,0.1},{0.1,0.1},{0.1,0.5}}, {CURVE,{0.1,0.9},{0.8,0.9},{0.8,0.5}}, {LINE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_b[] = {{MOVE,{0,0},{0,0},{0.2,0.0}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {CURVE,{0.2,1.0},{0.9,1.0},{0.9,0.5}}, {CURVE,{0.9,0.4},{0.2,0.4},{0.2,0.5}}};
static const Segment char_c[] = {{MOVE,{0,0},{0,0},{0.8,0.3}}, {CURVE,{0.1,0.1},{0.1,0.9},{0.8,0.7}}};
static const Segment char_d[] = {{MOVE,{0,0},{0,0},{0.8,0.0}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.8,0.4}}, {CURVE,{0.1,0.4},{0.1,1.0},{0.8,1.0}}};
static const Segment char_e[] = {{MOVE,{0,0},{0,0},{0.2,0.6}}, {LINE,{0,0},{0,0},{0.8,0.6}}, {CURVE,{0.8,0.2},{0.2,0.2},{0.2,0.7}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.8}}};
static const Segment char_f[] = {{MOVE,{0,0},{0,0},{0.8,0.1}}, {CURVE,{0.2,0.0},{0.2,0.4},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.0,0.4}}, {LINE,{0,0},{0,0},{0.5,0.4}}};
static const Segment char_g[] = {{MOVE,{0,0},{0,0},{0.8,0.7}}, {CURVE,{0.8,0.4},{0.2,0.4},{0.2,0.7}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.7}}, {LINE,{0,0},{0,0},{0.8,1.3}}, {CURVE,{0.8,1.5},{0.2,1.5},{0.1,1.3}}};
static const Segment char_h[] = {{MOVE,{0,0},{0,0},{0.2,0.0}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.2,0.7}}, {CURVE,{0.2,0.3},{0.8,0.3},{0.8,0.7}}, {LINE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_i[] = {{MOVE,{0,0},{0,0},{0.5,0.4}}, {LINE,{0,0},{0,0},{0.5,1.0}}, {MOVE,{0,0},{0,0},{0.5,0.15}}, {LINE,{0,0},{0,0},{0.5,0.2}}};
static const Segment char_j[] = {{MOVE,{0,0},{0,0},{0.5,0.15}}, {LINE,{0,0},{0,0},{0.5,0.2}}, {MOVE,{0,0},{0,0},{0.5,0.4}}, {LINE,{0,0},{0,0},{0.5,1.3}}, {CURVE,{0.5,1.5},{0.1,1.5},{0.1,1.3}}};
static const Segment char_k[] = {{MOVE,{0,0},{0,0},{0.2,0.0}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.7,0.4}}, {LINE,{0,0},{0,0},{0.2,0.7}}, {LINE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_l[] = {{MOVE,{0,0},{0,0},{0.5,0.0}}, {LINE,{0,0},{0,0},{0.5,1.0}}};
static const Segment char_m[] = {{MOVE,{0,0},{0,0},{0.15,0.4}}, {LINE,{0,0},{0,0},{0.15,1.0}}, {MOVE,{0,0},{0,0},{0.15,0.6}}, {CURVE,{0.15,0.4},{0.5,0.4},{0.5,0.6}}, {LINE,{0,0},{0,0},{0.5,1.0}}, {MOVE,{0,0},{0,0},{0.5,0.6}}, {CURVE,{0.5,0.4},{0.85,0.4},{0.85,0.6}}, {LINE,{0,0},{0,0},{0.85,1.0}}};
static const Segment char_n[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.2,0.6}}, {CURVE,{0.2,0.4},{0.8,0.4},{0.8,0.6}}, {LINE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_o[] = {{MOVE,{0,0},{0,0},{0.8,0.7}}, {CURVE,{0.8,0.4},{0.2,0.4},{0.2,0.7}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.7}}};
static const Segment char_p[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.2,1.4}}, {MOVE,{0,0},{0,0},{0.2,0.5}}, {CURVE,{0.2,0.4},{0.9,0.4},{0.9,0.7}}, {CURVE,{0.9,1.0},{0.2,1.0},{0.2,0.9}}};
static const Segment char_q[] = {{MOVE,{0,0},{0,0},{0.8,0.4}}, {CURVE,{0.8,0.4},{0.1,0.4},{0.1,0.7}}, {CURVE,{0.1,1.0},{0.8,1.0},{0.8,0.7}}, {LINE,{0,0},{0,0},{0.8,1.4}}};
static const Segment char_r[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {MOVE,{0,0},{0,0},{0.2,0.8}}, {CURVE,{0.2,0.4},{0.8,0.4},{0.8,0.5}}};
static const Segment char_s[] = {{MOVE,{0,0},{0,0},{0.8,0.45}}, {CURVE,{0.1,0.2},{0.1,0.7},{0.8,0.6}}, {CURVE,{0.8,0.5},{0.8,1.0},{0.1,0.85}}};
static const Segment char_t[] = {{MOVE,{0,0},{0,0},{0.5,0.1}}, {LINE,{0,0},{0,0},{0.5,1.0}}, {MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.8,0.4}}};
static const Segment char_u[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.4}}, {LINE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_v[] = {{MOVE,{0,0},{0,0},{0.1,0.4}}, {LINE,{0,0},{0,0},{0.5,1.0}}, {LINE,{0,0},{0,0},{0.9,0.4}}};
static const Segment char_w[] = {{MOVE,{0,0},{0,0},{0.1,0.4}}, {LINE,{0,0},{0,0},{0.3,1.0}}, {LINE,{0,0},{0,0},{0.5,0.6}}, {LINE,{0,0},{0,0},{0.7,1.0}}, {LINE,{0,0},{0,0},{0.9,0.4}}};
static const Segment char_x[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.8,1.0}}, {MOVE,{0,0},{0,0},{0.8,0.4}}, {LINE,{0,0},{0,0},{0.2,1.0}}};
static const Segment char_y[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {CURVE,{0.2,1.0},{0.8,1.0},{0.8,0.4}}, {LINE,{0,0},{0,0},{0.8,1.3}}, {CURVE,{0.8,1.5},{0.2,1.5},{0.1,1.3}}};
static const Segment char_z[] = {{MOVE,{0,0},{0,0},{0.2,0.4}}, {LINE,{0,0},{0,0},{0.8,0.4}}, {LINE,{0,0},{0,0},{0.2,1.0}}, {LINE,{0,0},{0,0},{0.8,1.0}}};
static const Segment char_sp[] = {}; 

static const Glyph glyph_a={char_a,4,0.85}, glyph_b={char_b,4,0.85}, glyph_c={char_c,2,0.75}, glyph_d={char_d,4,0.85}, glyph_e={char_e,4,0.75}, glyph_f={char_f,4,0.6}, glyph_g={char_g,5,0.85}, glyph_h={char_h,5,0.85}, glyph_i={char_i,4,0.35}, glyph_j={char_j,5,0.4}, glyph_k={char_k,5,0.8}, glyph_l={char_l,2,0.35}, glyph_m={char_m,8,1.2}, glyph_n={char_n,5,0.85}, glyph_o={char_o,3,0.85}, glyph_p={char_p,5,0.85}, glyph_q={char_q,4,0.85}, glyph_r={char_r,4,0.6}, glyph_s={char_s,3,0.7}, glyph_t={char_t,4,0.65}, glyph_u={char_u,3,0.85}, glyph_v={char_v,3,0.85}, glyph_w={char_w,5,1.2}, glyph_x={char_x,4,0.8}, glyph_y={char_y,4,0.85}, glyph_z={char_z,4,0.75}, glyph_space={char_sp,0,0.5};

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

uint32_t drawing_callback(uint32_t trigger_time, void *cb_arg) {
    if (!is_drawing) return 0;
    const Glyph* g = record_buffer[cur_char];

    if (g->segment_count == 0) {
        word_x_off += g->width;
        if (++cur_char >= record_len) { is_drawing = false; return 0; }
        return 1;
    }

    Segment s = g->segments[cur_seg];
    Point p = get_point(s, seg_start, t_prog);
    float wx = (p.x + word_x_off) * FONT_SIZE, wy = p.y * FONT_SIZE;

    if (s.type != MOVE) {
        float dx = (wx - last_abs.x) + sub_x, dy = (wy - last_abs.y) + sub_y;
        int8_t rx = (int8_t)roundf(dx), ry = (int8_t)roundf(dy);
        sub_x = dx - rx; sub_y = dy - ry;
        if (rx != 0 || ry != 0) host_mouse_send(&(report_mouse_t){.x = rx, .y = ry});
    } else { sub_x = 0; sub_y = 0; }

    last_abs = (Point){wx, wy};
    t_prog += DRAW_SPEED;

    if (t_prog >= 1.0f) {
        t_prog = 0; seg_start = s.dest;
        if (++cur_seg >= g->segment_count) {
            word_x_off += g->width; cur_seg = 0; seg_start = (Point){0,0};
            if (++cur_char >= record_len) { is_drawing = false; return 0; }
        }
    }
    return REPORT_INTERVAL;
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