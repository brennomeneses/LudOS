/* ============================================================
 * Lud OS — main.cpp
 * Ponto de entrada. Só orquestra a inicialização; a lógica de
 * cada parte mora nos módulos correspondentes.
 * ============================================================ */
#include <Arduino.h>
#include <lvgl.h>
#include "display_driver.h"
#include "touch_driver.h"
#include "theme.h"
#include "screens/screen_boot.h"

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("Lud OS - iniciando...");

    lv_init();
    display::init();
    touch::init();
    theme::init();

    screen_boot::create();

    Serial.println("Lud OS - boot OK, entrando no loop principal");
}

void loop() {
    // Avisa o LVGL de quanto tempo passou desde a última volta do loop.
    // Fazemos isso manualmente (em vez de confiar no LV_TICK_CUSTOM do
    // lv_conf.h) porque não há garantia de que o lv_conf.h esteja sendo
    // encontrado pelo compilador nesse ambiente — ver PLAN.md.
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    lv_tick_inc(now - last_tick);
    last_tick = now;

    lv_timer_handler();
    delay(5); // dá espaço pro watchdog/WiFi/etc respirarem
}
