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
    static uint32_t last_tick = 0;
    uint32_t now = millis();
    lv_tick_inc(now - last_tick);
    last_tick = now;

    lv_timer_handler();
    delay(5);
}
