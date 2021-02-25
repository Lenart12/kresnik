#ifndef INDEV_DRIVERS_H
#define INDEV_DRIVERS_H

#include <globals.h>

void button_init();
bool button_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
void button_feedback_cb(lv_indev_drv_t *drv, lv_event_t event);


void touchpad_init();
bool touchpad_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
void touchpad_feedback_cb(lv_indev_drv_t *drv, lv_event_t event);

#endif