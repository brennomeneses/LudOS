#include "theme.h"

namespace theme {

lv_style_t style_screen_bg;
lv_style_t style_card;
lv_style_t style_app_icon;
lv_style_t style_topbar;
lv_style_t style_text_title;
lv_style_t style_text_soft;

void init() {
    lv_style_init(&style_screen_bg);
    lv_style_set_bg_color(&style_screen_bg, lv_color_hex(PINK_100));
    lv_style_set_bg_opa(&style_screen_bg, LV_OPA_COVER);
    lv_style_set_border_width(&style_screen_bg, 0);
    lv_style_set_pad_all(&style_screen_bg, 0);
    lv_style_set_radius(&style_screen_bg, 0);

    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, lv_color_hex(WHITE));
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 10);
    lv_style_set_border_width(&style_card, 1);
    lv_style_set_border_color(&style_card, lv_color_hex(PINK_200));
    lv_style_set_pad_all(&style_card, 6);

    lv_style_init(&style_app_icon);
    lv_style_set_bg_color(&style_app_icon, lv_color_hex(WHITE));
    lv_style_set_bg_opa(&style_app_icon, LV_OPA_COVER);
    lv_style_set_radius(&style_app_icon, 11);
    lv_style_set_border_width(&style_app_icon, 0);
    lv_style_set_shadow_width(&style_app_icon, 0); /* ver nota de performance no .h */

    lv_style_init(&style_topbar);
    lv_style_set_bg_color(&style_topbar, lv_color_hex(PINK_50));
    lv_style_set_bg_opa(&style_topbar, LV_OPA_COVER);
    lv_style_set_border_width(&style_topbar, 0);
    lv_style_set_pad_all(&style_topbar, 4);
    lv_style_set_radius(&style_topbar, 0);

    lv_style_init(&style_text_title);
    lv_style_set_text_color(&style_text_title, lv_color_hex(INK));

    lv_style_init(&style_text_soft);
    lv_style_set_text_color(&style_text_soft, lv_color_hex(INK_SOFT));
}

}
