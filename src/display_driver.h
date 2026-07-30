#pragma once
#include <lvgl.h>

/* ============================================================
 * Lud OS — display_driver
 * Wrapper LovyanGFX + flush_cb pro LVGL.
 * ============================================================ */
namespace display {

constexpr uint16_t SCREEN_W = 240;
constexpr uint16_t SCREEN_H = 320;

void init();
void set_backlight(uint8_t percent); // 0-100

bool touch_read(uint16_t *x, uint16_t *y);

}