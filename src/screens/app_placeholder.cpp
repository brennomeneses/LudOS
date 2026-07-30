#include "app_placeholder.h"
#include "screen_home.h"
#include "../theme.h"
#include <lvgl.h>

namespace app_placeholder {

static void back_cb(lv_event_t *e) {
    (void)e;
    screen_home::create();
}

void show(const char *app_name, const char *icon_symbol) {
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &theme::style_screen_bg, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // ---- topbar: back button + título (padrão de TODAS as telas de app) ----
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
    lv_label_set_text(title, app_name);
    lv_obj_add_style(title, &theme::style_text_title, 0);
    lv_obj_align_to(title, back, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    // ---- corpo: placeholder "em construção" ----
    lv_obj_t *icon = lv_label_create(scr);
    lv_label_set_text(icon, icon_symbol);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(theme::PINK_400), 0);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *msg = lv_label_create(scr);
    lv_label_set_text(msg, "em construcao");
    lv_obj_add_style(msg, &theme::style_text_soft, 0);
    lv_obj_align_to(msg, icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    lv_scr_load(scr);
}

}
