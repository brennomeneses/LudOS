#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <lvgl.h>

namespace screen_manager {

void init();

lv_obj_t *current();

void replace(lv_obj_t *new_scr);

void register_timer(lv_timer_t *timer);

}

#endif
