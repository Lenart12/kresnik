#include <lv_gui.h>
#include <tasks.h>
#include <control.h>

lv_obj_t *main_screen;
lv_obj_t *kb_screen;
lv_obj_t *screensaver_screen;
lv_obj_t *time_label;
lv_obj_t *status_label;
lv_obj_t *header;
lv_obj_t *wifi_tab;
lv_obj_t *wifi_status;
lv_obj_t *wifi_list;
lv_obj_t *manual_control_tab;
lv_obj_t *status_tab;
lv_obj_t *temp_label;
lv_obj_t *env_chart;
lv_chart_series_t *env_data;
lv_obj_t *relay_label;

void lv_main(){
    main_screen = lv_cont_create(NULL, NULL);
    kb_screen = lv_cont_create(NULL, NULL);
    screensaver_screen = lv_cont_create(NULL, NULL);
    lv_disp_load_scr(main_screen);

    header = lv_cont_create(main_screen, NULL);
    lv_obj_set_width(header, lv_disp_get_hor_res(NULL));

    status_label = lv_label_create(header, NULL);
    lv_label_set_text(status_label, "");
    lv_obj_align(status_label, NULL, LV_ALIGN_IN_RIGHT_MID, -LV_DPI/10, 0);

    time_label = lv_label_create(header, NULL);
    tm time;
    getLocalTime(&time, 10);
    char ctime[10];
    strftime(ctime, 10, "%H:%M:%S", &time);
    lv_label_set_text(time_label, ctime);
    lv_obj_align(time_label, NULL, LV_ALIGN_IN_LEFT_MID, LV_DPI/10, 0);

    lv_cont_set_fit2(header, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_obj_set_pos(header, 0, 0);

    lv_obj_t * tv = lv_tabview_create(main_screen, NULL);
    lv_tabview_set_anim_time(tv, 0);
    lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL) - lv_obj_get_height(header));
    lv_obj_align(tv, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    status_tab = lv_tabview_add_tab(tv, LV_SYMBOL_HOME);
    lv_obj_t * settings_tab = lv_tabview_add_tab(tv, LV_SYMBOL_SETTINGS);
    wifi_tab = lv_tabview_add_tab(tv, LV_SYMBOL_WIFI);
    manual_control_tab = lv_tabview_add_tab(tv, LV_SYMBOL_EDIT);

    lv_create_status(status_tab);
    lv_create_wifi(wifi_tab);
    lv_create_settings(settings_tab);
    lv_create_manual_control(manual_control_tab);
}

void lv_create_status(lv_obj_t *tab){
    temp_label = lv_label_create(tab, NULL);
    String tempatures = 
    (String)"Gorilec: "+  '\n' + 
    "Hranilnik: " + '\n' +
    "Zunajnja: " + '\n' +
    "Mansarda: " + '\n' + 
    "Spalnice: " + '\n' + 
    "Pritlicje: " + '\n' +
    "Garaza: " + '\n' +
    "Solarni kolektorji: "  + '\n' +
    "Solarni tank: " + '\n' +
    "Izmenovalnik temp.: ";
    lv_label_set_text(temp_label, tempatures.c_str());
    lv_obj_align(temp_label, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 10);
    lv_obj_t *chart_l = lv_label_create(tab, NULL);
    lv_label_set_text(chart_l, "Zgodovina zunanje temperature");
    lv_obj_align(chart_l, temp_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    env_chart = lv_chart_create(tab, NULL);
    lv_obj_set_width(env_chart, lv_disp_get_hor_res(NULL) - 90);
    lv_chart_set_div_line_count(env_chart, 5, 24);
    lv_obj_set_height(env_chart, 100);
    lv_chart_set_range(env_chart, -20, 40);
    lv_chart_set_point_count(env_chart, 24);
    lv_obj_align(env_chart, chart_l, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    env_data = lv_chart_add_series(env_chart, LV_COLOR_RED);
    lv_chart_series_t *zero_d = lv_chart_add_series(env_chart, LV_COLOR_CYAN);
    lv_chart_init_points(env_chart, zero_d, 0);
    lv_chart_init_points(env_chart, env_data, 0);
    lv_chart_refresh(env_chart);
    chart_l = lv_label_create(env_chart, NULL);
    lv_label_set_text(chart_l, "T[C]\n40");
    lv_obj_align(chart_l, env_chart, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    chart_l = lv_label_create(env_chart, NULL);
    lv_label_set_text(chart_l, "-20\n  -24");
    lv_obj_align(chart_l, env_chart, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    chart_l = lv_label_create(env_chart, NULL);
    lv_label_set_text(chart_l, "0 T[H]");
    lv_obj_align(chart_l, env_chart, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
}

void lv_create_wifi(lv_obj_t *tab){
    wifi_status = lv_cont_create(tab, NULL);
    lv_obj_t *label = lv_label_create(wifi_status, NULL);
    lv_label_set_text(label, "Iskanje omrezji");

    lv_cont_set_fit2(wifi_status, LV_FIT_PARENT, LV_FIT_TIGHT);

    lv_obj_align(wifi_status, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    wifi_list = lv_list_create(tab, NULL);
    lv_obj_align(wifi_list, wifi_status, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    lv_task_create(wifi_search_task, 100, LV_TASK_PRIO_LOW, NULL);
}

void manualControlBtnmatrixCb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        uint32_t id = *(int*)lv_event_get_data();
        String txt = "Pressed " + (String)id;
        LV_LOG_INFO( txt.c_str() );
        uint8_t motor_id = id / 2;
        MotorDirection direction = (id % 2) ? MotorDirection::dir_open : MotorDirection::dir_close;
        motor_tickers[motor_id].detach();
        xSemaphoreGive(motor_mutexes[motor_id]);
        move_motor(motor_id, direction, 2000);
    }
}

void lv_create_manual_control(lv_obj_t *tab){
    lv_obj_t *btnm = lv_btnmatrix_create(tab, NULL);
    static const char* btn_map[] = {
        "M1 Zapri", "M1 Odpri", "\n",
        "M2 Zapri", "M2 Odpri", "\n",
        "M3 Zapri", "M3 Odpri", "\n",
        "M4 Zapri", "M4 Odpri", "\n", ""
    };
    lv_btnmatrix_set_map(btnm, btn_map);
    lv_obj_align(btnm, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(btnm, manualControlBtnmatrixCb);
}

void b_en_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.burner_enabled = lv_checkbox_is_checked(obj);
    }
}
void hwp_en_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.hot_water_pump_enabled = lv_checkbox_is_checked(obj);
    }
}
void c_en_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.circulator_pump_enabled = lv_checkbox_is_checked(obj);
    }
}
void uf_0_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.underfloor_pump_enabled[0] = lv_checkbox_is_checked(obj);
    }
}
void uf_1_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.underfloor_pump_enabled[1] = lv_checkbox_is_checked(obj);
    }
}
void uf_2_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.underfloor_pump_enabled[2] = lv_checkbox_is_checked(obj);
    }
}
void uf_3_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.underfloor_pump_enabled[3] = lv_checkbox_is_checked(obj);
    }
}

lv_obj_t *min_burner_temp;
lv_obj_t *min_burner_temp_l;
lv_obj_t *max_burner_temp;
lv_obj_t *max_burner_temp_l;

lv_obj_t *min_hwc_temp;
lv_obj_t *min_hwc_temp_l;
lv_obj_t *max_hwc_temp;
lv_obj_t *max_hwc_temp_l;

lv_obj_t *c_duration_l;

void min_b_temp_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.boiler_min_temp = lv_slider_get_value(obj);

        String txt = (String)"Min. temp. kotla 10-"+config.control.boiler_min_temp+'-'+(config.control.boiler_max_temp - 10.00f);
        lv_label_set_text(min_burner_temp_l, txt.c_str());
        txt = (String)"Max. temp. kotla " + (config.control.boiler_min_temp + 10.00f) + '-' + config.control.boiler_max_temp+"-90";
        lv_label_set_text(max_burner_temp_l, txt.c_str());

        lv_slider_set_range(max_burner_temp, config.control.boiler_min_temp + 10.00f, 90);
        lv_slider_set_value(max_burner_temp, config.control.boiler_max_temp, LV_ANIM_OFF);
    }
}
void max_b_temp_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.boiler_max_temp = lv_slider_get_value(obj);

        String txt = (String)"Min. temp. kotla 10-"+config.control.boiler_min_temp+'-'+(config.control.boiler_max_temp - 10.00f);
        lv_label_set_text(min_burner_temp_l, txt.c_str());
        txt = (String)"Max. temp. kotla " + (config.control.boiler_min_temp + 10.00f) + '-' + config.control.boiler_max_temp+"-90";
        lv_label_set_text(max_burner_temp_l, txt.c_str());

        lv_slider_set_range(min_burner_temp, 10.00f, config.control.boiler_max_temp - 10.00f);
        lv_slider_set_value(min_burner_temp, config.control.boiler_min_temp, LV_ANIM_OFF);

        txt = (String)"Max. temp. hranilnika " + (config.control.hot_water_min_temp + 5.00f) + '-' +  config.control.hot_water_max_temp + '-' + (config.control.boiler_max_temp - 5.00f);
        lv_label_set_text(max_hwc_temp_l, txt.c_str());
        lv_slider_set_range(max_hwc_temp, config.control.hot_water_min_temp + 5.00f, config.control.boiler_max_temp - 5.00f);
        lv_slider_set_value(max_hwc_temp, config.control.hot_water_max_temp, LV_ANIM_OFF);
    }
}

void min_hwc_temp_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.hot_water_min_temp = lv_slider_get_value(obj);

        String txt = (String)"Min. temp. hranilnika 5-"+config.control.hot_water_min_temp+'-'+(config.control.hot_water_max_temp - 5.00f);
        lv_label_set_text(min_hwc_temp_l, txt.c_str());

        txt = (String)"Max. temp. hranilnika " + (config.control.hot_water_min_temp + 5.00f) + '-' +  config.control.hot_water_max_temp + '-' + (config.control.boiler_max_temp - 5.00f);
        lv_label_set_text(max_hwc_temp_l, txt.c_str());
        lv_slider_set_range(max_hwc_temp, config.control.hot_water_min_temp + 5.00f, config.control.boiler_max_temp - 5.00f);
        lv_slider_set_value(max_hwc_temp, config.control.hot_water_max_temp, LV_ANIM_OFF);
    }
}
void max_hwc_temp_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.hot_water_max_temp = lv_slider_get_value(obj);

        String txt = (String)"Max. temp. hranilnika " + (config.control.hot_water_min_temp + 5.00f) + '-' +  config.control.hot_water_max_temp + '-' + (config.control.boiler_max_temp - 5.00f);
        lv_label_set_text(max_hwc_temp_l, txt.c_str());

        txt = (String)"Min. temp. hranilnika 5-"+config.control.hot_water_min_temp+'-'+(config.control.hot_water_max_temp - 5.00f);
        lv_label_set_text(min_hwc_temp_l, txt.c_str());
        lv_slider_set_range(min_hwc_temp, 5.00f, config.control.hot_water_max_temp - 5.00f);
        lv_slider_set_value(min_hwc_temp, config.control.hot_water_min_temp, LV_ANIM_OFF);
    }
}

void c_duration_cb(lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        config.control.circulator_duration = lv_slider_get_value(obj);
        lv_label_set_text(c_duration_l, ((String)"Trajanje krozenja: " + config.control.circulator_duration).c_str());
    }
}

void lv_create_settings(lv_obj_t *settings_tab){
    lv_obj_t *burner_enabled_cb = lv_checkbox_create(settings_tab, NULL);
    lv_checkbox_set_checked(burner_enabled_cb, config.control.burner_enabled);
    lv_checkbox_set_text(burner_enabled_cb, "Gorilec");
    lv_obj_set_event_cb(burner_enabled_cb, b_en_cb);
    lv_obj_align(burner_enabled_cb, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 10);

    min_burner_temp_l = lv_label_create(settings_tab, NULL);
    min_burner_temp =  lv_slider_create(settings_tab, NULL);
    lv_obj_set_event_cb(min_burner_temp, min_b_temp_cb);

    String txt = (String)"Min. temp. kotla 10-"+config.control.boiler_min_temp+'-'+(config.control.boiler_max_temp - 10.00f);
    lv_label_set_text(min_burner_temp_l, txt.c_str());
    lv_slider_set_range(min_burner_temp, 10.00f, config.control.boiler_max_temp - 10.00f);
    lv_slider_set_value(min_burner_temp, config.control.boiler_min_temp, LV_ANIM_OFF);

    lv_obj_align(min_burner_temp_l, burner_enabled_cb, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_obj_align(min_burner_temp, min_burner_temp_l, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    max_burner_temp_l = lv_label_create(settings_tab, NULL);
    max_burner_temp =  lv_slider_create(settings_tab, NULL);
    lv_obj_set_event_cb(max_burner_temp, max_b_temp_cb);

    txt = (String)"Max. temp. kotla " + (config.control.boiler_min_temp + 10.00f) + '-' + config.control.boiler_max_temp+"-90";
    lv_label_set_text(max_burner_temp_l, txt.c_str());
    lv_slider_set_range(max_burner_temp, config.control.boiler_min_temp + 10.00f, 90);
    lv_slider_set_value(max_burner_temp, config.control.boiler_max_temp, LV_ANIM_OFF);

    lv_obj_align(max_burner_temp_l, min_burner_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_obj_align(max_burner_temp, max_burner_temp_l, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t *hot_water_pump_enabled = lv_checkbox_create(settings_tab, NULL);
    lv_checkbox_set_checked(hot_water_pump_enabled, config.control.hot_water_pump_enabled);
    lv_checkbox_set_text(hot_water_pump_enabled, "Hranilnik");
    lv_obj_set_event_cb(hot_water_pump_enabled, hwp_en_cb);
    lv_obj_align(hot_water_pump_enabled, max_burner_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    min_hwc_temp_l = lv_label_create(settings_tab, NULL);
    min_hwc_temp =  lv_slider_create(settings_tab, NULL);
    lv_obj_set_event_cb(min_hwc_temp, min_hwc_temp_cb);

    txt = (String)"Min. temp. hranilnika 5-"+config.control.hot_water_min_temp+'-'+(config.control.hot_water_max_temp - 5.00f);
    lv_label_set_text(min_hwc_temp_l, txt.c_str());
    lv_slider_set_range(min_hwc_temp, 5.00f, config.control.hot_water_max_temp - 5.00f);
    lv_slider_set_value(min_hwc_temp, config.control.hot_water_min_temp, LV_ANIM_OFF);

    lv_obj_align(min_hwc_temp_l, hot_water_pump_enabled, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_obj_align(min_hwc_temp, min_hwc_temp_l, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    max_hwc_temp_l = lv_label_create(settings_tab, NULL);
    max_hwc_temp =  lv_slider_create(settings_tab, NULL);
    lv_obj_set_event_cb(max_hwc_temp, max_hwc_temp_cb);

    txt = (String)"Max. temp. hranilnika " + (config.control.hot_water_min_temp + 5.00f) + '-' +  config.control.hot_water_max_temp + '-' + (config.control.boiler_max_temp - 5.00f);
    lv_label_set_text(max_hwc_temp_l, txt.c_str());
    lv_slider_set_range(max_hwc_temp, config.control.hot_water_min_temp + 5.00f, config.control.boiler_max_temp - 5.00f);
    lv_slider_set_value(max_hwc_temp, config.control.hot_water_max_temp, LV_ANIM_OFF);

    lv_obj_align(max_hwc_temp_l, min_hwc_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_obj_align(max_hwc_temp, max_hwc_temp_l, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t *circulation_pump_enabled = lv_checkbox_create(settings_tab, NULL);
    lv_checkbox_set_checked(circulation_pump_enabled, config.control.circulator_pump_enabled);
    lv_checkbox_set_text(circulation_pump_enabled, "Krozenje vode");
    lv_obj_set_event_cb(circulation_pump_enabled, c_en_cb);
    lv_obj_align(circulation_pump_enabled, max_hwc_temp, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    c_duration_l = lv_label_create(settings_tab, NULL);
    lv_label_set_text(c_duration_l, ((String)"Trajanje krozenja: " + config.control.circulator_duration).c_str());
    lv_obj_align(c_duration_l, circulation_pump_enabled, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t *c_duration = lv_slider_create(settings_tab, NULL);
    lv_slider_set_range(c_duration, 1, 20);
    lv_slider_set_value(c_duration, config.control.circulator_duration, LV_ANIM_OFF);
    lv_obj_set_event_cb(c_duration, c_duration_cb);
    lv_obj_align(c_duration, c_duration_l, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t *uf_0_en = lv_checkbox_create(settings_tab, NULL);
    lv_checkbox_set_checked(uf_0_en, config.control.underfloor_pump_enabled[0]);
    lv_checkbox_set_text(uf_0_en, "Mansarda");
    lv_obj_set_event_cb(uf_0_en, uf_0_cb);
    lv_obj_align(uf_0_en, c_duration, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t *uf_1_en = lv_checkbox_create(settings_tab, NULL);
    lv_checkbox_set_checked(uf_1_en, config.control.underfloor_pump_enabled[1]);
    lv_checkbox_set_text(uf_1_en, "Spalnice");
    lv_obj_set_event_cb(uf_1_en, uf_1_cb);
    lv_obj_align(uf_1_en, uf_0_en, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t *uf_2_en = lv_checkbox_create(settings_tab, NULL);
    lv_checkbox_set_checked(uf_2_en, config.control.underfloor_pump_enabled[2]);
    lv_checkbox_set_text(uf_2_en, "Pritlicje");
    lv_obj_set_event_cb(uf_2_en, uf_2_cb);
    lv_obj_align(uf_2_en, uf_1_en, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    lv_obj_t *uf_3_en = lv_checkbox_create(settings_tab, NULL);
    lv_checkbox_set_checked(uf_3_en, config.control.underfloor_pump_enabled[3]);
    lv_checkbox_set_text(uf_3_en, "Garaza");
    lv_obj_set_event_cb(uf_3_en, uf_3_cb);
    lv_obj_align(uf_3_en, uf_2_en, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
}
