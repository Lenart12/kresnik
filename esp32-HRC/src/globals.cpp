#include <globals.h>

SemaphoreHandle_t i2c_semaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t timing_semaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t tempature_semaphore= xSemaphoreCreateBinary();

bool minute_update;

lv_task_t *update_task_h;
lv_task_t *request_temp_task_h;
lv_task_t *screensaver_task_h;

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
Ticker conversion_ticker;
TFT_eSPI tft = TFT_eSPI();

lv_disp_buf_t disp_buf;
lv_color_t buf[LV_HOR_RES_MAX * 10];
lv_point_t button_targets[4];

Ticker motor_tickers[4];
xSemaphoreHandle motor_semaphores[4] = {
    xSemaphoreCreateBinary(),
    xSemaphoreCreateBinary(),
    xSemaphoreCreateBinary(),
    xSemaphoreCreateBinary()
};

float boiler_temp[24] = {0};
float hot_water_container_temp[24] = {0};
float enviroment_temp[24] = {0};
float underfloor_temp[4][24] = {0};
float solar_collector_temp[24] = {0};
float solar_tank_temp[24] = {0};
float heat_exchanger_temp[24] = {0};
