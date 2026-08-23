#include "screen_invaders.h"
#include "screen_home.h"
#include "../theme.h"
#include "../screen_manager.h"
#include "../games/space_invaders.h"
#include <Arduino.h>
#include <lvgl.h>

namespace screen_invaders {

constexpr uint16_t CANVAS_W = 240;
constexpr uint16_t CANVAS_H = 248;

static uint8_t canvas_buf[((CANVAS_W / 2) + 1) * CANVAS_H + 4 * 16];

static lv_obj_t *score_lbl;
static lv_obj_t *lives_lbl;

static void back_cb(lv_event_t *e) {
    (void)e;
    space_invaders::stop();
    screen_home::create();
}

static void left_press_cb(lv_event_t *e) {
    (void)e;
    space_invaders::move_player(-1);
}

static void left_release_cb(lv_event_t *e) {
    (void)e;
    space_invaders::release_player(-1);
}

static void right_press_cb(lv_event_t *e) {
    (void)e;
    space_invaders::move_player(1);
}

static void right_release_cb(lv_event_t *e) {
    (void)e;
    space_invaders::release_player(1);
}

static void fire_cb(lv_event_t *e) {
    (void)e;
    space_invaders::fire();
}

void create() {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &theme::style_screen_bg, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *topbar = lv_obj_create(scr);
    lv_obj_add_style(topbar, &theme::style_topbar, 0);
    lv_obj_set_size(topbar, LV_PCT(100), 26);
    lv_obj_align(topbar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(topbar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *back = lv_btn_create(topbar);
    lv_obj_set_size(back, 20, 20);
    lv_obj_align(back, LV_ALIGN_LEFT_MID, 4, 0);
    lv_obj_set_style_radius(back, 10, 0);
    lv_obj_set_style_bg_color(back, lv_color_hex(theme::WHITE), 0);
    lv_obj_add_event_cb(back, back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *back_lbl = lv_label_create(back);
    lv_label_set_text(back_lbl, LV_SYMBOL_LEFT);
    lv_obj_center(back_lbl);

    lv_obj_t *title = lv_label_create(topbar);
    lv_label_set_text(title, "Invaders");
    lv_obj_add_style(title, &theme::style_text_title, 0);
    lv_obj_align_to(title, back, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lives_lbl = lv_label_create(topbar);
    lv_label_set_text(lives_lbl, LV_SYMBOL_OK " " LV_SYMBOL_OK " " LV_SYMBOL_OK);
    lv_obj_set_style_text_font(lives_lbl, &lv_font_montserrat_12, 0);
    lv_obj_add_style(lives_lbl, &theme::style_text_title, 0);
    lv_obj_align(lives_lbl, LV_ALIGN_RIGHT_MID, -4, 0);

    score_lbl = lv_label_create(topbar);
    lv_label_set_text(score_lbl, "score 0000");
    lv_obj_set_style_text_font(score_lbl, &lv_font_montserrat_12, 0);
    lv_obj_add_style(score_lbl, &theme::style_text_title, 0);
    lv_obj_align_to(score_lbl, lives_lbl, LV_ALIGN_OUT_LEFT_MID, -6, 0);

    lv_obj_t *canvas_obj = lv_canvas_create(scr);
    lv_obj_align(canvas_obj, LV_ALIGN_TOP_MID, 0, 26);
    lv_canvas_set_buffer(canvas_obj, canvas_buf, CANVAS_W, CANVAS_H, LV_IMG_CF_INDEXED_4BIT);

    lv_canvas_set_palette(canvas_obj, 0,  lv_palette_main(LV_PALETTE_BLUE_GREY));
    lv_canvas_set_palette(canvas_obj, 1,  lv_palette_main(LV_PALETTE_GREY));
    lv_canvas_set_palette(canvas_obj, 2,  lv_palette_main(LV_PALETTE_PINK));
    lv_canvas_set_palette(canvas_obj, 3,  lv_palette_main(LV_PALETTE_DEEP_PURPLE));
    lv_canvas_set_palette(canvas_obj, 4,  lv_palette_main(LV_PALETTE_PURPLE));
    lv_canvas_set_palette(canvas_obj, 5,  lv_palette_main(LV_PALETTE_DEEP_PURPLE));
    lv_canvas_set_palette(canvas_obj, 6,  lv_palette_main(LV_PALETTE_RED));
    lv_canvas_set_palette(canvas_obj, 7,  lv_palette_main(LV_PALETTE_BROWN));

    lv_obj_t *ctrl_bar = lv_obj_create(scr);
    lv_obj_remove_style_all(ctrl_bar);
    lv_obj_set_size(ctrl_bar, LV_PCT(100), 32);
    lv_obj_align(ctrl_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(ctrl_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(ctrl_bar, 6, 0);
    lv_obj_clear_flag(ctrl_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *left_btn = lv_btn_create(ctrl_bar);
    lv_obj_set_size(left_btn, 78, 28);
    lv_obj_set_style_bg_color(left_btn, lv_color_hex(theme::PINK_400), 0);
    lv_obj_set_style_radius(left_btn, 8, 0);
    lv_obj_add_event_cb(left_btn, left_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(left_btn, left_release_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_t *left_lbl = lv_label_create(left_btn);
    lv_label_set_text(left_lbl, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(left_lbl, lv_color_hex(theme::WHITE), 0);
    lv_obj_center(left_lbl);

    lv_obj_t *fire_btn = lv_btn_create(ctrl_bar);
    lv_obj_set_size(fire_btn, 78, 28);
    lv_obj_set_style_bg_color(fire_btn, lv_color_hex(theme::PINK_600), 0);
    lv_obj_set_style_radius(fire_btn, 8, 0);
    lv_obj_add_event_cb(fire_btn, fire_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *fire_lbl = lv_label_create(fire_btn);
    lv_label_set_text(fire_lbl, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_color(fire_lbl, lv_color_hex(theme::WHITE), 0);
    lv_obj_center(fire_lbl);

    lv_obj_t *right_btn = lv_btn_create(ctrl_bar);
    lv_obj_set_size(right_btn, 78, 28);
    lv_obj_set_style_bg_color(right_btn, lv_color_hex(theme::PINK_400), 0);
    lv_obj_set_style_radius(right_btn, 8, 0);
    lv_obj_add_event_cb(right_btn, right_press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(right_btn, right_release_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_t *right_lbl = lv_label_create(right_btn);
    lv_label_set_text(right_lbl, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(right_lbl, lv_color_hex(theme::WHITE), 0);
    lv_obj_center(right_lbl);

    screen_manager::replace(scr);

    space_invaders::start(canvas_obj, score_lbl, lives_lbl);
}

}
