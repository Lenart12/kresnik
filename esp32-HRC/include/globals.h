#ifndef GLOBALS_H
#define GLOBALS_H

#include <defines.h>

#include <Wire.h>
#include <PCF8574_WDDR.h>
extern PCF8574_WDDR status_expander;
extern PCF8574_WDDR thermo_expander;
extern PCF8574_WDDR relay0_expander;
extern PCF8574_WDDR relay1_expander;

#include <control.h>
extern Control control;

#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

#include <OneWire.h>
#include <DallasTemperature.h>
extern OneWire ds2482_0;
extern OneWire ds2482_1;
extern DallasTemperature tempature_1;
extern DallasTemperature tempature_2;

#include <Ticker.h>
#include <TFT_eSPI.h>
extern Ticker tick;
extern TFT_eSPI tft;

#include <lvgl.h>
extern lv_disp_buf_t disp_buf;
extern lv_color_t buf[LV_HOR_RES_MAX * 10];
extern lv_point_t button_targets[4];

#endif