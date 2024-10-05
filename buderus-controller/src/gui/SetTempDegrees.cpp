#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME SetTempDegrees

uint16_t temp_x, temp_y;

float steps[] = {0.5, 1, 2, 5, 10};
int current_step = 3;

void draw_current_setting(TFT_eSPI& _tft, float val) {
    _tft.setTextColor(TFT_BUDERUS_BLUE, TFT_WHITE, true);
    _tft.setTextDatum(MC_DATUM);
    _tft.fillSmoothCircle(temp_x, temp_y, _tft.fontHeight() * 2, TFT_WHITE);
    char _buffer[10];
    sprintf(_buffer, "%.1f", val);
    _tft.drawString(_buffer, temp_x, temp_y);
}

void draw_current_step(TFT_eSPI& _tft) {
    _tft.setTextColor(TFT_BLACK, TFT_BUDERUS_BLUE, true);
    _tft.setTextDatum(TC_DATUM);
    char _buffer[12];
    sprintf(_buffer, " +- %.1f ", steps[current_step]);
    _tft.drawString(_buffer, _tft.width() / 2, 70);

    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE, true);
    sprintf(_buffer, " -%.1f ", steps[current_step]);
    _tft.drawString(_buffer, _tft.width() / 8, temp_y);

    _tft.setTextColor(TFT_YELLOW, TFT_BUDERUS_BLUE, true);
    sprintf(_buffer, " +%.1f ", steps[current_step]);
    _tft.drawString(_buffer, _tft.width() / 8 * 7, temp_y);
}

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString("Nastavitev temperature", _tft.width() / 2, 10);
    _tft.drawString(_set_temp.name, _tft.width() / 2, 40);

    temp_x = _tft.width() / 2;
    temp_y = _tft.height() / 3 * 2;

    draw_current_setting(_tft, *_set_temp.setting);
    draw_current_step(_tft);

    _tft.setTextDatum(BL_DATUM);
    _tft.setTextColor(TFT_CYAN, TFT_BUDERUS_BLUE);
    _tft.drawString("OK", 5, _tft.height() - 5);
}

void GUI_METHOD(update)() {}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {
    if (y < 60) return;
    if (y < 80 && abs(x - _tft.width() / 2) < 40) return GUI_METHOD(on_btn_press)(Button::Black);
    if (abs(y - temp_y) < 20) {
        if (x < _tft.width() / 4) return GUI_METHOD(on_btn_press)(Button::White);
        if (x > _tft.width() / 4 * 3) return GUI_METHOD(on_btn_press)(Button::Yellow);
    }
    if (y > _tft.height() - 50 && x < 50) return change_page(Page::Settings);
}

void GUI_METHOD(on_btn_press)(Button btn) {
    switch (btn) {
        case Button::Black:
            current_step += 1;
            if (current_step >= sizeof(steps) / sizeof(steps[0])) {
                current_step = 0;
            }
            draw_current_step(_tft);
            break;
        case Button::White:
            *_set_temp.setting -= steps[current_step];
            *_set_temp.setting = max(*_set_temp.setting, _set_temp.min);
            draw_current_setting(_tft, *_set_temp.setting);
            break;
        case Button::Yellow:
            *_set_temp.setting += steps[current_step];
            *_set_temp.setting = min(*_set_temp.setting, _set_temp.max);
            draw_current_setting(_tft, *_set_temp.setting);
            break;
        case Button::Blue:
            change_page(Page::Settings);
            break;
    }
}
