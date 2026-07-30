#include "screen_boot.h"
#include "screen_lock.h"
#include "../theme.h"
#include <lvgl.h>

namespace screen_boot {

static lv_obj_t *scr;
static lv_obj_t *logo;

static void logo_pop_anim_cb(void *var, int32_t v) {
    lv_obj_set_style_translate_y((lv_obj_t *)var, v, 0);
}

static void goto_lock_cb(lv_timer_t *timer) {
    lv_timer_del(timer);
    screen_lock::create();
}

void create() {
    scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &theme::style_screen_bg, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(theme::PINK_200), 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // "logo" — bolha rosa com a letra L, igual ao mockup
    logo = lv_obj_create(scr);
    lv_obj_set_size(logo, 64, 64);
    lv_obj_set_style_radius(logo, 26, 0);
    lv_obj_set_style_bg_color(logo, lv_color_hex(theme::PINK_400), 0);
    lv_obj_set_style_border_width(logo, 0, 0);
    lv_obj_align(logo, LV_ALIGN_CENTER, 0, -30);
    lv_obj_clear_flag(logo, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *logo_lbl = lv_label_create(logo);
    lv_label_set_text(logo_lbl, "L");
    lv_obj_set_style_text_color(logo_lbl, lv_color_hex(theme::WHITE), 0);
    lv_obj_set_style_text_font(logo_lbl, &lv_font_montserrat_20, 0);
    lv_obj_center(logo_lbl);

    // animação sutil de "flutuar" — mesmo efeito do @keyframes pop do mockup
    static lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, logo);
    lv_anim_set_values(&a, 0, -4);
    lv_anim_set_time(&a, 700);
    lv_anim_set_playback_time(&a, 700);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, logo_pop_anim_cb);
    lv_anim_start(&a);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Lud OS");
    lv_obj_add_style(title, &theme::style_text_title, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align_to(title, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);

    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text(sub, "powered by ESP32");
    lv_obj_add_style(sub, &theme::style_text_soft, 0);
    lv_obj_set_style_text_font(sub, &lv_font_montserrat_12, 0);
    lv_obj_align_to(sub, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    lv_obj_t *bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 90, 4);
    lv_obj_set_style_bg_color(bar, lv_color_hex(theme::PINK_50), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(theme::PINK_400), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 2, LV_PART_MAIN);
    lv_obj_align_to(bar, sub, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);

    lv_anim_t bar_anim;
    lv_anim_init(&bar_anim);
    lv_anim_set_var(&bar_anim, bar);
    lv_anim_set_values(&bar_anim, 0, 100);
    lv_anim_set_time(&bar_anim, 1400);
    lv_anim_set_exec_cb(&bar_anim, [](void *var, int32_t v) {
        lv_bar_set_value((lv_obj_t *)var, v, LV_ANIM_OFF);
    });
    lv_anim_start(&bar_anim);

    lv_scr_load(scr);

    lv_timer_t *t = lv_timer_create(goto_lock_cb, 1700, NULL);
    lv_timer_set_repeat_count(t, 1);
}

} // namespace screen_boot
