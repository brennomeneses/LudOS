#include "screen_manager.h"
#include <Arduino.h>

namespace screen_manager {

static lv_obj_t *current_scr = nullptr;
static lv_timer_t *current_timer = nullptr;

static void delete_later_cb(void *obj) {
    if (obj != nullptr) {
        lv_obj_del((lv_obj_t *)obj);
    }
}

void init() {
    current_scr = nullptr;
    current_timer = nullptr;
}

lv_obj_t *current() {
    return current_scr;
}

void register_timer(lv_timer_t *timer) {
    current_timer = timer;
}

void replace(lv_obj_t *new_scr) {
    if (new_scr == nullptr) {
        return;
    }
    lv_obj_t *old = current_scr;
    lv_timer_t *old_timer = current_timer;
    current_scr = new_scr;
    current_timer = nullptr;
    lv_scr_load(new_scr);
    if (old_timer != nullptr) {
        lv_timer_del(old_timer);
    }
    if (old != nullptr && old != new_scr) {
        Serial.printf("screen_manager: deferring delete of %p\n", old);
        lv_async_call(delete_later_cb, old);
    }
}

}
