#pragma once

#include <lvgl.h>

namespace space_invaders {

void start(lv_obj_t *canvas, lv_obj_t *score_lbl, lv_obj_t *lives_lbl);
void stop();
void reset();
void move_player(int8_t dir);
void release_player(int8_t dir);
void fire();

}
