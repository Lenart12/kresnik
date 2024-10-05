#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME SetTempSensor

void draw_temps(TFT_eSPI& _tft, float* temps) {
    uint16_t y = 40;
    uint16_t x = _tft.width() - 5;
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE, true);
    _tft.setTextDatum(TR_DATUM);
    char buffer[10];
    for (int i = 0; i < 3; i++) {
        sprintf(buffer, "  %.1fC ", temps[i]);
        _tft.drawString(buffer, x, y);
        y += _tft.fontHeight() + 5;
    }
}

uint8_t active_sensor_idx = 0;

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString(_set_sensor.name, _tft.width() / 2, 10);
    _tft.setTextDatum(TL_DATUM);
    
    active_sensor_idx = *_set_sensor.setting;

    uint16_t y = 40;
    uint16_t max_height = _tft.height() - _tft.fontHeight() - 10;

    int temp0_count = _temp_0.GetDeviceCount();
    
    for (int i = 0; i < 3; i++) {
        if (i == active_sensor_idx) {
            _tft.fillRect(8, y - 2, _tft.width() - 16, _tft.fontHeight() + 4, TFT_WHITE);
            _tft.setTextColor(TFT_BUDERUS_BLUE, TFT_WHITE);
        } else {
            _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
        }
        int ds_id = 0;
        int dev_id = i;
        if (i >= temp0_count) {
            ds_id = 1;
            dev_id -= temp0_count;
        }
        char buffer[10];
        sprintf(buffer, "%d-%d.%d", i, ds_id, dev_id);
        _tft.drawString(buffer, 10, y);
        y += _tft.fontHeight() + 5;
    }

    _tft.setTextDatum(TR_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.drawString("^ ", _tft.width() - 5, 20);
    _tft.setTextDatum(BR_DATUM);
    _tft.setTextColor(TFT_YELLOW, TFT_BUDERUS_BLUE);
    _tft.drawString("v ", _tft.width() - 5, _tft.height() - 10);
    _tft.setTextDatum(BL_DATUM);
    _tft.setTextColor(TFT_CYAN, TFT_BUDERUS_BLUE);
    _tft.drawString("OK", 5, _tft.height() - 5);
}

void GUI_METHOD(update)() {
    if (run_every(1000)) return;
    float t[3];
    int j = 0;
    for (int i = 0; i < _temp_0.GetDeviceCount(); i++) {
        if (j >= 3) break;
        t[j++] = _temp_0.GetTemperatureC(i);
    }
    for (int i = 0; i < _temp_1.GetDeviceCount(); i++) {
        if (j >= 3) break;
        t[j++] = _temp_1.GetTemperatureC(i);
    }

    draw_temps(_tft, t);
}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {
    if (y < 32 && x > _tft.width() - 50) return GUI_METHOD(on_btn_press)(Button::White);
    if (y < 40) return;
    if (y > _tft.height() - 50) {
        if (x < 50) return GUI_METHOD(on_btn_press)(Button::Blue);
        if (x > _tft.width() - 50) return GUI_METHOD(on_btn_press)(Button::Yellow);
        return;
    }
    int touch_idx = (y - 40) / (_tft.fontHeight() + 5);
    if (touch_idx < 0 || touch_idx > 2) return;
    active_sensor_idx = touch_idx;
    *_set_sensor.setting = active_sensor_idx;
    change_page(Page::RedrawPage);
}

void GUI_METHOD(on_btn_press)(Button btn) {

    switch (btn) {
        case Button::Black:
            break;
        case Button::White:
            active_sensor_idx--;
            if (active_sensor_idx > 2) {
                active_sensor_idx = 2;
            }
            *_set_sensor.setting = active_sensor_idx;
            change_page(Page::RedrawPage);
            break;
        case Button::Yellow:
            active_sensor_idx++;
            if (active_sensor_idx > 2) {
                active_sensor_idx = 0;
            }
            *_set_sensor.setting = active_sensor_idx;
            change_page(Page::RedrawPage);
            break;
        case Button::Blue:
            change_page(Page::Settings);
            break;
    }
}