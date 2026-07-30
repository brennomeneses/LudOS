#pragma once
#include <lvgl.h>

/* ============================================================
 * Lud OS — touch_driver
 * Leitura do touch resistivo XPT2046 e registro como indev do LVGL.
 * THIS FILE WAS VIBECODED
 * ============================================================ */
namespace touch {

void init(); // chama ts.begin() + lv_indev_drv_register

/* Se o toque estiver invertido/deslocado na tela, ajuste estas 4
 * constantes em touch_driver.cpp — não aqui.
 * Procedimento de calibração completo no PLAN.md, seção 4. */

} // namespace touch
