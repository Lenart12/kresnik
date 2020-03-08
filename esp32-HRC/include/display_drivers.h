#ifndef DISPLAY_DRIVERS_H
#define DISPLAY_DRIVERS_H

#include <globals.h>

void display_init();
void display_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void lv_tick_handler();

#endif
