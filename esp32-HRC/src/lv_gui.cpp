#include <lv_gui.h>
#include <tasks.h>

lv_obj_t *main_screen;
lv_obj_t *kb_screen;
lv_obj_t *time_label;
lv_obj_t *status_label;
lv_obj_t *header;
lv_obj_t *wifi_tab;

void lv_main(){
    main_screen = lv_cont_create(NULL, NULL);
    kb_screen = lv_cont_create(NULL, NULL);
    lv_disp_load_scr(main_screen);

    header = lv_cont_create(main_screen, NULL);
    lv_obj_set_width(header, lv_disp_get_hor_res(NULL));

    status_label = lv_label_create(header, NULL);
    lv_label_set_text(status_label, "");
    lv_obj_align(status_label, NULL, LV_ALIGN_IN_RIGHT_MID, -LV_DPI/10, 0);

    time_label = lv_label_create(header, NULL);
    lv_label_set_text(time_label, time_client.getFormattedTime().c_str());
    lv_obj_align(time_label, NULL, LV_ALIGN_IN_LEFT_MID, LV_DPI/10, 0);

    lv_cont_set_fit2(header, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_pos(header, 0, 0);

    lv_obj_t * tv = lv_tabview_create(main_screen, NULL);
    lv_tabview_set_sliding(tv, false);
    lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL) - lv_obj_get_height(header));
    lv_obj_align(tv, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_t * status_tab = lv_tabview_add_tab(tv, LV_SYMBOL_HOME);
    wifi_tab = lv_tabview_add_tab(tv, LV_SYMBOL_WIFI);
    lv_obj_t * motor_tab = lv_tabview_add_tab(tv, "Ventili");
    lv_obj_t * relay_tab = lv_tabview_add_tab(tv, "Releji");


    lv_create_wifi(wifi_tab);
}

void lv_create_wifi(lv_obj_t *tab){
    lv_obj_t *preloader = lv_preload_create(tab, NULL);
    lv_obj_align(preloader, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *srch = lv_label_create(tab, NULL);
    lv_label_set_text(srch, "Iskanje omrezji");
    lv_obj_align(srch, preloader, LV_ALIGN_OUT_TOP_MID, 0, 0);

    WiFi.scanNetworks(true);
    lv_task_create(wifi_search_task, 100, LV_TASK_PRIO_LOW, NULL);
}
