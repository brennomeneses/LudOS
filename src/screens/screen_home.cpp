#include "screen_home.h"
#include "app_placeholder.h"
#include "../theme.h"
#include <Arduino.h>
#include <lvgl.h>

namespace screen_home {

static lv_obj_t *scr;
static lv_obj_t *clock_lbl_top;
static lv_obj_t *clock_lbl_widget;

struct AppDef {
    const char *name;
    const char *symbol;
};

/* Lista de apps do sistema. Pra add um app novo,
 * basta acrescentar uma linha aqui e criar o handler dele — ver
 * PLAN.md, seção 7.
 */
static const AppDef APPS[] = {
    {"Telefone",    LV_SYMBOL_CALL},
    {"Mensagens",   LV_SYMBOL_ENVELOPE},
    {"Notas",       LV_SYMBOL_EDIT},
    {"Galeria",     LV_SYMBOL_IMAGE},
    {"Calculadora", LV_SYMBOL_LIST},
    {"Wi-Fi",       LV_SYMBOL_WIFI},
    {"Ajustes",     LV_SYMBOL_SETTINGS},
    {"Invaders",    LV_SYMBOL_PLAY},
};
static const int APP_COUNT = sizeof(APPS) / sizeof(APPS[0]);

static void app_icon_cb(lv_event_t *e) {
    intptr_t idx = (intptr_t)lv_event_get_user_data(e);
    const AppDef &app = APPS[idx];

    // TODO: quando "Invaders" tiver o jogo de verdade, troque esta
    // linha por: if (idx == 7) { game::start(); return; }
    app_placeholder::show(app.name, app.symbol);
}

static lv_obj_t *create_app_icon(lv_obj_t *parent, const AppDef &app, int idx, bool dock_style) {
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 44, dock_style ? 34 : 44);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, app_icon_cb, LV_EVENT_CLICKED, (void *)(intptr_t)idx);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *glyph = lv_obj_create(cont);
    lv_obj_add_style(glyph, &theme::style_app_icon, 0);
    lv_obj_set_size(glyph, 30, 30);
    lv_obj_clear_flag(glyph, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(glyph, LV_OBJ_FLAG_CLICKABLE); // clique passa pro pai (cont)

    lv_obj_t *icon_lbl = lv_label_create(glyph);
    lv_label_set_text(icon_lbl, app.symbol);
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(theme::PINK_400), 0);
    lv_obj_center(icon_lbl);

    if (!dock_style) {
        lv_obj_t *name_lbl = lv_label_create(cont);
        lv_label_set_text(name_lbl, app.name);
        lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_12, 0);
        lv_obj_add_style(name_lbl, &theme::style_text_title, 0);
    }

    return cont;
}

static void clock_timer_cb(lv_timer_t *timer) {
    (void)timer;
    static char buf[6];
    uint32_t s = millis() / 1000;
    snprintf(buf, sizeof(buf), "%02lu:%02lu", (s / 60) % 24, s % 60);
    lv_label_set_text(clock_lbl_top, buf);
    lv_label_set_text(clock_lbl_widget, buf);
}

void create() {
    scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &theme::style_screen_bg, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // ---- status bar ----
    lv_obj_t *status = lv_obj_create(scr);
    lv_obj_remove_style_all(status);
    lv_obj_set_size(status, LV_PCT(100), 18);
    lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_pad_hor(status, 8, 0);
    lv_obj_clear_flag(status, LV_OBJ_FLAG_SCROLLABLE);

    clock_lbl_top = lv_label_create(status);
    lv_label_set_text(clock_lbl_top, "--:--");
    lv_obj_set_style_text_font(clock_lbl_top, &lv_font_montserrat_12, 0);
    lv_obj_align(clock_lbl_top, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *status_icons = lv_label_create(status);
    lv_label_set_text(status_icons, LV_SYMBOL_WIFI "  " LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_font(status_icons, &lv_font_montserrat_12, 0);
    lv_obj_align(status_icons, LV_ALIGN_RIGHT_MID, 0, 0);

    // ---- widget de relógio ----
    lv_obj_t *widget = lv_obj_create(scr);
    lv_obj_add_style(widget, &theme::style_card, 0);
    lv_obj_set_size(widget, LV_PCT(92), 40);
    lv_obj_align_to(widget, status, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
    lv_obj_clear_flag(widget, LV_OBJ_FLAG_SCROLLABLE);

    clock_lbl_widget = lv_label_create(widget);
    lv_label_set_text(clock_lbl_widget, "--:--");
    lv_obj_set_style_text_font(clock_lbl_widget, &lv_font_montserrat_20, 0);
    lv_obj_add_style(clock_lbl_widget, &theme::style_text_title, 0);
    lv_obj_align(clock_lbl_widget, LV_ALIGN_CENTER, 0, -4);

    lv_obj_t *widget_sub = lv_label_create(widget);
    lv_label_set_text(widget_sub, "Lud OS");
    lv_obj_set_style_text_font(widget_sub, &lv_font_montserrat_12, 0);
    lv_obj_add_style(widget_sub, &theme::style_text_soft, 0);
    lv_obj_align_to(widget_sub, clock_lbl_widget, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    // ---- grid de apps ----
    lv_obj_t *grid = lv_obj_create(scr);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, LV_PCT(100), 190);
    lv_obj_align_to(grid, widget, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(grid, 12, 0);
    lv_obj_set_style_pad_row(grid, 12, 0);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    for (int i = 0; i < APP_COUNT; i++) {
        create_app_icon(grid, APPS[i], i, false);
    }

    // ---- dock ----
    lv_obj_t *dock = lv_obj_create(scr);
    lv_obj_add_style(dock, &theme::style_card, 0);
    lv_obj_set_style_bg_color(dock, lv_color_hex(theme::PINK_50), 0);
    lv_obj_set_size(dock, LV_PCT(92), 40);
    lv_obj_align(dock, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_set_flex_flow(dock, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dock, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(dock, LV_OBJ_FLAG_SCROLLABLE);

    static const int DOCK_IDX[4] = {0, 1, 7, 6}; // Telefone, Mensagens, Invaders, Ajustes
    for (int i = 0; i < 4; i++) {
        create_app_icon(dock, APPS[DOCK_IDX[i]], DOCK_IDX[i], true);
    }

    lv_scr_load(scr);

    static lv_timer_t *t = lv_timer_create(clock_timer_cb, 1000, NULL);
    (void)t;
    clock_timer_cb(NULL);
}

}
