#ifndef LV_GUI_H
#define LV_GUI_H

#include <globals.h>

extern lv_obj_t *main_screen;
extern lv_obj_t *kb_screen;
extern lv_obj_t *time_label;
extern lv_obj_t *status_label;
extern lv_obj_t *header;
extern lv_obj_t *wifi_tab;
extern lv_obj_t *status_tab;
extern lv_obj_t *temp_label;
extern lv_obj_t *env_chart;
extern lv_chart_series_t *env_data;
extern lv_obj_t *relay_label;

void lv_main();
void lv_create_status(lv_obj_t *tab);
void lv_create_wifi(lv_obj_t *tab);
void lv_create_settings(lv_obj_t *settings);

#endif