#include "screen_lock.h"
#include "screen_home.h"
#include "../theme.h"
#include <Arduino.h>
#include <lvgl.h>
#include <time.h>

namespace screen_lock {

static lv_obj_t *scr;
static lv_obj_t *time_lbl;
static lv_obj_t *date_lbl;

static void tap_cb(lv_event_t *e) {
    (void)e;
    screen_home::create();
}

static void clock_timer_cb(lv_timer_t *timer) {
    (void)timer;
    // TODO: quando tiver RTC/NTP (app Wi-Fi implementado), troque
    // isto por horário real. Por enquanto usa millis() de exemplo.
    static char buf[6];
    uint32_t s = millis() / 1000;
    snprintf(buf, sizeof(buf), "%02lu:%02lu", (s / 60) % 24, s % 60);
    lv_label_set_text(time_lbl, buf);
}

void create() {
    scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &theme::style_screen_bg, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(theme::PINK_200), 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr, tap_cb, LV_EVENT_CLICKED, NULL);

    time_lbl = lv_label_create(scr);
    lv_label_set_text(time_lbl, "--:--");
    lv_obj_add_style(time_lbl, &theme::style_text_title, 0);
    lv_obj_set_style_text_font(time_lbl, &lv_font_montserrat_20, 0);
    lv_obj_align(time_lbl, LV_ALIGN_TOP_MID, 0, 60);

    date_lbl = lv_label_create(scr);
    lv_label_set_text(date_lbl, "toque para desbloquear");
    lv_obj_add_style(date_lbl, &theme::style_text_soft, 0);
    lv_obj_set_style_text_font(date_lbl, &lv_font_montserrat_12, 0);
    lv_obj_align_to(date_lbl, time_lbl, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, LV_SYMBOL_UP " deslize / toque " LV_SYMBOL_UP);
    lv_obj_add_style(hint, &theme::style_text_soft, 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -18);

    lv_scr_load(scr);

    static lv_timer_t *t = lv_timer_create(clock_timer_cb, 1000, NULL);
    (void)t;
}

} // namespace screen_lock
