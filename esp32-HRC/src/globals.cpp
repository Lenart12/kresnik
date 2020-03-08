#include <globals.h>

SemaphoreHandle_t i2c_semaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t timing_semaphore= xSemaphoreCreateBinary();

uint32_t last_conversion_request = millis();
uint32_t last_control_update = millis();

PCF8574_WDDR status_expander;
PCF8574_WDDR thermo_expander;
PCF8574_WDDR relay0_expander;
PCF8574_WDDR relay1_expander;

Config config;

WiFiUDP ntp_UDP;
NTPClient time_client(ntp_UDP);

OneWire ds2482_0(DS2482_0);
OneWire ds2482_1(DS2482_1);
DallasTemperature tempature_1(&ds2482_0);
DallasTemperature tempature_2(&ds2482_1);

Ticker tick;
TFT_eSPI tft = TFT_eSPI();

lv_disp_buf_t disp_buf;
lv_color_t buf[LV_HOR_RES_MAX * 10];
lv_point_t button_targets[4];