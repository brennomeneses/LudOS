#include "touch_driver.h"
#include "display_driver.h"

namespace touch {

static void read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    uint16_t tx, ty;
    bool pressed = display::touch_read(&tx, &ty);

    if (pressed) {
        data->point.x = tx;
        data->point.y = ty;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void init() {
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = read_cb;
    lv_indev_drv_register(&indev_drv);
}

}