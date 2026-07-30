#include "display_driver.h"
#include <TFT_eSPI.h>
#include <SPI.h>

namespace display {

/* ROTATION: 0=retrado, 1=paisagem, 2=retrado invertido, 3=paisagem invertido */
constexpr uint8_t ROTATION = 1;

static TFT_eSPI tft_inst = TFT_eSPI();

/* Touch XPT2046 no barramento VSPI separado do display.
 * CLK=25, MOSI=32, MISO=39, CS=33, IRQ=36 (não usado, polling). */
static SPIClass touchSPI(VSPI_HOST);

/* Funções de baixo nível para ler o XPT2046 via SPI custom. */
static void touch_write_byte(uint8_t b) {
    touchSPI.transfer(b);
}
static uint16_t touch_read_adc(uint8_t cmd) {
    touchSPI.beginTransaction(SPISettings(2500000, MSBFIRST, SPI_MODE0));
    digitalWrite(33, LOW);
    touchSPI.transfer(cmd);
    uint16_t val = touchSPI.transfer16(0) >> 3;
    digitalWrite(33, HIGH);
    touchSPI.endTransaction();
    return val & 0x0FFF;
}

bool touch_read(uint16_t *x, uint16_t *y) {
    // Verifica pressão (Z)
    int16_t z1 = touch_read_adc(0xB1);
    int16_t z2 = touch_read_adc(0xC1);
    int z = z1 + 4095 - z2;
    if (z < 400) return false;

    // Lê X e Y (eixos trocados: raw X → tela Y, raw Y → tela X)
    int16_t rx = touch_read_adc(0xD1);  // raw Y do chip → tela X
    int16_t ry = touch_read_adc(0x91);  // raw X do chip → tela Y

    *x = map(rx, 240, 3800, 0, SCREEN_W - 1);
    *y = map(ry, 200, 3700, SCREEN_H - 1, 0);  // Y invertido
    if (*x < 0) *x = 0; if (*x >= SCREEN_W) *x = SCREEN_W - 1;
    if (*y < 0) *y = 0; if (*y >= SCREEN_H) *y = SCREEN_H - 1;
    return true;
}

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_W * 40];
static lv_color_t buf2[SCREEN_W * 40];

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft_inst.startWrite();
    tft_inst.setAddrWindow(area->x1, area->y1, w, h);
    tft_inst.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft_inst.endWrite();

    lv_disp_flush_ready(drv);
}

void init() {
    Serial.printf("display init: rot=%u\n", ROTATION);
    tft_inst.init();
    tft_inst.setRotation(ROTATION);
    tft_inst.fillScreen(TFT_BLACK);
    tft_inst.fillScreen(TFT_BLACK);  // limpa buffer stale
    ledcSetup(0, 5000, 8);
    ledcAttachPin(21, 0);
    ledcWrite(0, 255);
    Serial.println("display init OK");

    // Init touch SPI bus separado (VSPI_HOST)
    touchSPI.begin(25, 39, 32, 33);
    pinMode(33, OUTPUT);
    digitalWrite(33, HIGH);

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCREEN_W * 40);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_W;
    disp_drv.ver_res = SCREEN_H;
    disp_drv.flush_cb = flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

void set_backlight(uint8_t percent) {
    if (percent > 100) percent = 100;
    ledcWrite(0, (percent * 255) / 100);
}

} // namespace display