#ifndef GLOBALS_H
#define GLOBALS_H

#include <defines.h>

#include <Arduino.h>
extern SemaphoreHandle_t i2c_mutex;

extern SemaphoreHandle_t timing_mutex;
extern SemaphoreHandle_t tempature_mutex;

extern bool minute_update;

extern lv_task_t *update_task_h;
extern lv_task_t *request_temp_task_h;
extern lv_task_t *screensaver_task_h;

#include <Wire.h>
#include <PCF8574_WDDR.h>
extern PCF8574_WDDR status_expander;
extern PCF8574_WDDR thermo_expander;
extern PCF8574_WDDR relay0_expander;
extern PCF8574_WDDR relay1_expander;

#include <control.h>
struct WiFi_login{
    char ssid[32];
    char passwd[64];
};

struct Config{
    WiFi_login wifi_login;
    uint32_t burner_seconds;
    uint32_t solar_minutes;
    Control control;
};
extern Config config;

#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
extern WiFiUDP ntp_UDP;
extern NTPClient time_client;

#include <OneWire.h>
#include <DallasTemperature.h>
extern OneWire ds2482_0;
extern OneWire ds2482_1;
extern DallasTemperature tempature_1;
extern DallasTemperature tempature_2;

#include <Ticker.h>
#include <TFT_eSPI.h>
extern Ticker tick;
extern Ticker conversion_ticker;
extern TFT_eSPI tft;

#include <lvgl.h>
extern lv_disp_buf_t disp_buf;
extern lv_color_t buf[LV_HOR_RES_MAX * 10];
extern lv_point_t button_targets[4];

extern Ticker motor_tickers[4];
extern xSemaphoreHandle motor_mutexes[4];

extern float boiler_temp[24];
extern float hot_water_container_temp[24];
extern float enviroment_temp[24];
extern float underfloor_temp[4][24];
extern float solar_collector_temp[24];
extern float solar_tank_temp[24];
extern float heat_exchanger_temp[24];

#endif