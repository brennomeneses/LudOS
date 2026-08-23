#include "space_invaders.h"
#include <Arduino.h>

namespace space_invaders {

constexpr uint16_t CANVAS_W = 240;
constexpr uint16_t CANVAS_H = 248;

constexpr int SPRITE_SCALE = 2;
constexpr int SPRITE_SIZE = 8 * SPRITE_SCALE;

constexpr int GRID_COLS = 11;
constexpr int GRID_ROWS = 5;
constexpr int TOTAL_ENEMIES = GRID_COLS * GRID_ROWS;

constexpr int CELL_W = SPRITE_SIZE + 4;
constexpr int CELL_H = SPRITE_SIZE + 6;
constexpr int GRID_W = GRID_COLS * CELL_W;
constexpr int GRID_START_X = (CANVAS_W - GRID_W) / 2;
constexpr int GRID_START_Y = 20;

constexpr int PLAYER_W = SPRITE_SIZE;
constexpr int PLAYER_H = SPRITE_SIZE;
constexpr int PLAYER_Y = 232;
constexpr int PLAYER_SPEED = 4 * SPRITE_SCALE;

constexpr int BULLET_W = 2 * SPRITE_SCALE;
constexpr int BULLET_H = 5 * SPRITE_SCALE;
constexpr int PLAYER_BULLET_SPEED = 4;
constexpr int ENEMY_BULLET_SPEED = 2;
constexpr int MAX_ENEMY_BULLETS = 3;

constexpr int STEP_PX = 4 * SPRITE_SCALE;
constexpr int DROP_PX = 6 * SPRITE_SCALE;

enum PaletteIdx {
    PAL_BG     = 0,
    PAL_STAR   = 1,
    PAL_PLAYER = 2,
    PAL_E0     = 3,
    PAL_E1     = 4,
    PAL_E2     = 5,
    PAL_BULLET = 6,
    PAL_UI     = 7
};

static uint8_t canvas_w;
static uint8_t canvas_h;
static uint8_t *canvas_data;

static uint8_t pal_idx(uint8_t pal) { return pal; }

static void set_px(int16_t x, int16_t y, uint8_t idx) {
    if (x < 0 || x >= canvas_w || y < 0 || y >= canvas_h) return;
    int byte_off = ((canvas_w + 1) >> 1) * y + (x >> 1);
    uint8_t shift = (x & 1) ? 0 : 4;
    canvas_data[byte_off] = (canvas_data[byte_off] & ~(0xF << shift)) | ((idx & 0xF) << shift);
}

static void fill_bg(uint8_t idx) {
    int row_bytes = (canvas_w + 1) >> 1;
    int pixels_per_byte = 2;
    uint8_t val = (idx << 4) | (idx & 0xF);
    memset(canvas_data, val, row_bytes * canvas_h);
}

static void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t idx) {
    for (int16_t dy = 0; dy < h; dy++) {
        for (int16_t dx = 0; dx < w; dx++) {
            set_px(x + dx, y + dy, idx);
        }
    }
}

static void draw_sprite(int16_t x, int16_t y, const uint8_t *data, uint8_t sprite_w, uint8_t sprite_h, uint8_t idx) {
    for (uint8_t row = 0; row < sprite_h; row++) {
        for (uint8_t col = 0; col < sprite_w; col++) {
            if (data[row] & (1 << (sprite_w - 1 - col))) {
                for (int sy = 0; sy < SPRITE_SCALE; sy++) {
                    for (int sx = 0; sx < SPRITE_SCALE; sx++) {
                        set_px(x + col * SPRITE_SCALE + sx, y + row * SPRITE_SCALE + sy, idx);
                    }
                }
            }
        }
    }
}

struct Entity {
    int16_t x, y;
    int16_t w, h;
    bool alive;
};

static lv_obj_t *canvas = nullptr;
static lv_obj_t *score_lbl = nullptr;
static lv_obj_t *lives_lbl = nullptr;
static lv_timer_t *game_timer = nullptr;

static Entity player;
static Entity player_bullet;
static Entity enemy_bullets[MAX_ENEMY_BULLETS];
static Entity enemies[GRID_ROWS][GRID_COLS];

static uint16_t score;
static uint8_t  lives;
static bool     game_over;

static int32_t step_accum;
static int32_t step_interval;
static int8_t  enemy_dir;
static int     alive_count;
static bool    held_left;
static bool    held_right;

static const uint8_t sprite_player[8] = {
    0b00011000,
    0b00111100,
    0b01111110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111
};

static const uint8_t sprite_octopus[8] = {
    0b00011000,
    0b00111100,
    0b01101110,
    0b11011011,
    0b11111111,
    0b00100100,
    0b01011010,
    0b10100101
};

static const uint8_t sprite_crab[8] = {
    0b01000010,
    0b00100100,
    0b01111110,
    0b11011011,
    0b11111111,
    0b10111101,
    0b10100101,
    0b00100100
};

static const uint8_t sprite_squid[8] = {
    0b00011000,
    0b00111100,
    0b01111110,
    0b11011011,
    0b11111111,
    0b01011010,
    0b10000001,
    0b01000010
};

static void draw_stars() {
    static const uint8_t sx[] = {15,50,85,120,160,200,225,30,70,110,150,190,210,40,100,170};
    static const uint8_t sy[] = {8,30,55,18,42,12,60,75,90,38,70,85,50,100,110,120};
    for (uint8_t i = 0; i < 16; i++) {
        set_px(sx[i], sy[i], pal_idx(PAL_STAR));
    }
}

static void draw_all() {
    fill_bg(pal_idx(PAL_BG));
    draw_stars();

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (!enemies[r][c].alive) continue;
            uint8_t idx;
            const uint8_t *spr;
            if (r == 0)      { idx = PAL_E0; spr = sprite_octopus; }
            else if (r <= 2) { idx = PAL_E1; spr = sprite_crab;    }
            else              { idx = PAL_E2; spr = sprite_squid;   }
            draw_sprite(enemies[r][c].x + (enemies[r][c].w - SPRITE_SIZE) / 2, enemies[r][c].y, spr, 8, 8, pal_idx(idx));
        }
    }

    draw_sprite(player.x + (player.w - SPRITE_SIZE) / 2, player.y, sprite_player, 8, 8, pal_idx(PAL_PLAYER));

    if (player_bullet.alive)
        draw_rect(player_bullet.x, player_bullet.y, player_bullet.w, player_bullet.h, pal_idx(PAL_BULLET));

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].alive)
            draw_rect(enemy_bullets[i].x, enemy_bullets[i].y, enemy_bullets[i].w, enemy_bullets[i].h, pal_idx(PAL_BULLET));
    }

    lv_obj_invalidate(canvas);
}

static void spawn_enemies() {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            enemies[r][c].alive = true;
            enemies[r][c].w = CELL_W;
            enemies[r][c].h = SPRITE_SIZE;
            enemies[r][c].x = GRID_START_X + c * CELL_W + (CELL_W - SPRITE_SIZE) / 2;
            enemies[r][c].y = GRID_START_Y + r * CELL_H;
        }
    }
}

static int compute_speed() {
    int interval = 500 - (TOTAL_ENEMIES - alive_count) * 7;
    return interval < 80 ? 80 : interval;
}

static void step_enemies() {
    int shift = enemy_dir * STEP_PX;
    bool hit_edge = false;

    for (int r = 0; r < GRID_ROWS && !hit_edge; r++) {
        for (int c = 0; c < GRID_COLS && !hit_edge; c++) {
            if (!enemies[r][c].alive) continue;
            int nx = enemies[r][c].x + shift;
            if (nx < 0 || nx + enemies[r][c].w > CANVAS_W) {
                hit_edge = true;
            }
        }
    }

    if (hit_edge) {
        enemy_dir = -enemy_dir;
        for (int r = 0; r < GRID_ROWS; r++) {
            for (int c = 0; c < GRID_COLS; c++) {
                if (!enemies[r][c].alive) continue;
                enemies[r][c].y += DROP_PX;
            }
        }
    } else {
        for (int r = 0; r < GRID_ROWS; r++) {
            for (int c = 0; c < GRID_COLS; c++) {
                if (!enemies[r][c].alive) continue;
                enemies[r][c].x += shift;
            }
        }
    }
}

static uint8_t fire_col = 0;

static void maybe_fire_enemy() {
    int attempts = 0;
    while (attempts < GRID_COLS) {
        bool any_alive = false;
        for (int r = 0; r < GRID_ROWS; r++) {
            if (enemies[r][fire_col].alive) { any_alive = true; break; }
        }
        if (any_alive) break;
        fire_col = (fire_col + 1) % GRID_COLS;
        attempts++;
    }
    if (attempts >= GRID_COLS) return;

    for (int r = GRID_ROWS - 1; r >= 0; r--) {
        if (!enemies[r][fire_col].alive) continue;
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (enemy_bullets[i].alive) continue;
            enemy_bullets[i].alive = true;
            enemy_bullets[i].x = enemies[r][fire_col].x + enemies[r][fire_col].w / 2 - 1;
            enemy_bullets[i].y = enemies[r][fire_col].y + enemies[r][fire_col].h;
            enemy_bullets[i].w = BULLET_W;
            enemy_bullets[i].h = BULLET_H;
            fire_col = (fire_col + 1) % GRID_COLS;
            return;
        }
        break;
    }
}

static void advance_bullets() {
    if (player_bullet.alive) {
        player_bullet.y -= PLAYER_BULLET_SPEED;
        if (player_bullet.y + player_bullet.h < 0) {
            player_bullet.alive = false;
        }
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].alive) continue;
        enemy_bullets[i].y += ENEMY_BULLET_SPEED;
        if (enemy_bullets[i].y > CANVAS_H) {
            enemy_bullets[i].alive = false;
        }
    }
}

static bool collides(const Entity &a, const Entity &b) {
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}

static void check_collisions() {
    if (player_bullet.alive) {
        for (int r = 0; r < GRID_ROWS; r++) {
            for (int c = 0; c < GRID_COLS; c++) {
                if (!enemies[r][c].alive) continue;
                if (collides(player_bullet, enemies[r][c])) {
                    enemies[r][c].alive = false;
                    player_bullet.alive = false;
                    alive_count--;
                    score += (r == 0) ? 30 : (r <= 2) ? 20 : 10;
                    step_interval = compute_speed();
                    if (alive_count == 0) {
                        game_over = true;
                    }
                    return;
                }
            }
        }
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].alive) continue;
        if (collides(enemy_bullets[i], player)) {
            enemy_bullets[i].alive = false;
            lives--;
            if (lives == 0) {
                game_over = true;
            }
            return;
        }
    }

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (!enemies[r][c].alive) continue;
            if (enemies[r][c].y + enemies[r][c].h >= PLAYER_Y) {
                game_over = true;
                return;
            }
        }
    }
}

static void update_hud() {
    static char buf[16];
    snprintf(buf, sizeof(buf), "score %04u", score);
    lv_label_set_text(score_lbl, buf);
    if (lives == 3)      lv_label_set_text(lives_lbl, LV_SYMBOL_OK " " LV_SYMBOL_OK " " LV_SYMBOL_OK);
    else if (lives == 2) lv_label_set_text(lives_lbl, LV_SYMBOL_OK " " LV_SYMBOL_OK);
    else if (lives == 1) lv_label_set_text(lives_lbl, LV_SYMBOL_OK);
    else                 lv_label_set_text(lives_lbl, "");
}

static void game_tick(lv_timer_t *t) {
    (void)t;
    if (game_over) return;

    if (held_left && player.x > 0) {
        player.x -= PLAYER_SPEED;
    }
    if (held_right && player.x + player.w < CANVAS_W) {
        player.x += PLAYER_SPEED;
    }

    step_accum += 16;
    if (step_accum >= step_interval) {
        step_accum = 0;
        step_enemies();
        maybe_fire_enemy();
    }

    advance_bullets();
    check_collisions();
    draw_all();
    update_hud();
}

void start(lv_obj_t *c, lv_obj_t *sl, lv_obj_t *ll) {
    canvas = c;
    score_lbl = sl;
    lives_lbl = ll;
    canvas_w = CANVAS_W;
    canvas_h = CANVAS_H;
    lv_img_dsc_t *dsc = lv_canvas_get_img(canvas);
    canvas_data = (uint8_t *)dsc->data + sizeof(lv_color32_t) * 16;
    held_left = false;
    held_right = false;
    reset();
    game_timer = lv_timer_create(game_tick, 16, NULL);
}

void stop() {
    game_over = true;
}

void reset() {
    game_over = false;
    score = 0;
    lives = 3;
    enemy_dir = 1;
    step_accum = 0;
    step_interval = 500;
    alive_count = TOTAL_ENEMIES;
    fire_col = 0;

    player.x = CANVAS_W / 2 - PLAYER_W / 2;
    player.y = PLAYER_Y;
    player.w = PLAYER_W;
    player.h = PLAYER_H;
    player.alive = true;

    player_bullet.alive = false;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        enemy_bullets[i].alive = false;
    }

    spawn_enemies();
    draw_all();
    update_hud();
}

void move_player(int8_t dir) {
    if (dir < 0)      held_left  = true;
    else if (dir > 0) held_right = true;
}

void release_player(int8_t dir) {
    if (dir < 0)      held_left  = false;
    else if (dir > 0) held_right = false;
}

void fire() {
    if (game_over || player_bullet.alive) return;
    player_bullet.alive = true;
    player_bullet.x = player.x + player.w / 2 - BULLET_W / 2;
    player_bullet.y = player.y - BULLET_H;
    player_bullet.w = BULLET_W;
    player_bullet.h = BULLET_H;
}

}
