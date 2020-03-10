#ifndef TASKS_H
#define TASKS_H

#include <globals.h>

void wifi_status_task(lv_task_t *task);
void wifi_search_task(lv_task_t *task);
void wifi_choose(lv_obj_t * obj, lv_event_t event);
void wifi_connect(lv_obj_t * obj, lv_event_t event);

void time_display_task(lv_task_t *task);

void status_display_task(lv_task_t *task);

void screensaver_task(lv_task_t *task);

void tempature_request_task(lv_task_t *task);
void tempature_read_task();
void tempature_shift_task(lv_task_t *task);

void control_update_task(lv_task_t *task);
void control_minute_update_task(lv_task_t *t);
#endif