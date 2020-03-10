#ifndef LOGGING_H
#define LOGGING_H

#include <lvgl.h>
#include <Arduino.h>
#include <globals.h>

void logging_cb(lv_log_level_t level, const char * file, uint32_t line, const char * dsc){
    if(level == LV_LOG_LEVEL_ERROR){
        status_expander.digitalWrite(Status_led::error, ON);
        lv_task_enable(false);
        digitalWrite(TFT_LED, ON);
        relay0_expander.write(0xFF);
        relay1_expander.write(0xFF);
        thermo_expander.write(0xFF);
        lv_obj_t *scr =  lv_cont_create(NULL, NULL);
        lv_disp_load_scr(scr);
        lv_obj_t * label = lv_label_create(scr, NULL);
        String err = LV_SYMBOL_WARNING +  (String)" File: " + file + '#' + line + "\n " + dsc;
        lv_label_set_text(label, err.c_str());
        lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
    }
    else if(level == LV_LOG_LEVEL_WARN)  Serial.print("WARNING: ");
    else if(level == LV_LOG_LEVEL_INFO)  Serial.print("INFO: ");
    else if(level == LV_LOG_LEVEL_TRACE) Serial.print("TRACE: ");

    Serial.print("File: ");
    Serial.print(file);

    char line_str[8];
    sprintf(line_str,"%d", line);
    Serial.print("#");
    Serial.print(line_str);

    Serial.print(": ");
    Serial.print(dsc);
    Serial.print("\n");
}

#endif