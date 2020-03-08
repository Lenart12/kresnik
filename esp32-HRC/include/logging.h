#ifndef LOGGING_H
#define LOGGING_H

#include <lvgl.h>
#include <Arduino.h>

void logging_cb(lv_log_level_t level, const char * file, uint32_t line, const char * dsc){
    if(level == LV_LOG_LEVEL_ERROR) Serial.print("ERROR: ");
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