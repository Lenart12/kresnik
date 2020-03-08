#ifndef LV_GUI_H
#define LV_GUI_H

#include <globals.h>

extern lv_obj_t *main_screen;
extern lv_obj_t *kb_screen;
extern lv_obj_t *time_label;
extern lv_obj_t *status_label;
extern lv_obj_t *header;
extern lv_obj_t *wifi_tab;

void lv_main();
void lv_create_status(lv_obj_t *tab);
void lv_create_wifi(lv_obj_t *tab);

#endif