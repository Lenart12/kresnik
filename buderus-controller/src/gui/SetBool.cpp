#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME SetBool

void draw_enable_disable(TFT_eSPI &_tft, bool enabled) {
    _tft.setTextDatum(MC_DATUM);
    uint16_t fg, bg;

    if (!enabled) {
        fg = TFT_BUDERUS_BLUE;
        bg = TFT_WHITE;
        _tft.fillSmoothCircle(_tft.width() / 4, _tft.height() / 2, _tft.fontHeight() * 3, bg);
    } else {
        fg = TFT_WHITE;
        bg = TFT_BUDERUS_BLUE;
        _tft.fillCircle(_tft.width() / 4, _tft.height() / 2, _tft.fontHeight() * 3, bg);
        _tft.drawSmoothCircle(_tft.width() / 4, _tft.height() / 2, _tft.fontHeight() * 3, fg, bg);
    }
    _tft.setTextColor(fg, bg);
    _tft.drawString("Izklop", _tft.width() / 4, _tft.height() / 2);
    if (enabled) {
        fg = TFT_BUDERUS_BLUE;
        bg = TFT_YELLOW;
        _tft.fillSmoothCircle(_tft.width() / 4 * 3, _tft.height() / 2, _tft.fontHeight() * 3, bg);
    } else {
        fg = TFT_YELLOW;
        bg = TFT_BUDERUS_BLUE;
        _tft.fillCircle(_tft.width() / 4 * 3, _tft.height() / 2, _tft.fontHeight() * 3, bg);
        _tft.drawSmoothCircle(_tft.width() / 4 * 3, _tft.height() / 2, _tft.fontHeight() * 3, fg, bg);
    }
    _tft.setTextColor(fg, bg);
    _tft.drawString("Vklop", _tft.width() / 4 * 3, _tft.height() / 2);
}

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE);
    _tft.setTextSize(2);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString(_set_bool.name, _tft.width() / 2, 10);
    _tft.setTextDatum(TL_DATUM);

    draw_enable_disable(_tft, *_set_bool.setting);

    _tft.setTextDatum(BL_DATUM);
    _tft.setTextColor(TFT_CYAN);
    _tft.drawString("OK", 5, _tft.height() - 5);
}

void GUI_METHOD(update)() {}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {
    if (x < 50 && y > _tft.height() - 50) change_page(Page::Settings);
    if (y < 100 || y > _tft.height() - 100) return;
    *_set_bool.setting = x > _tft.width() / 2;
    draw_enable_disable(_tft, *_set_bool.setting);
}

void GUI_METHOD(on_btn_press)(Button btn) {
    switch (btn) {
        case Button::White:
            *_set_bool.setting = false;
            draw_enable_disable(_tft, *_set_bool.setting);
            break;
        case Button::Yellow:
            *_set_bool.setting = true;
            draw_enable_disable(_tft, *_set_bool.setting);
            break;
        case Button::Blue:
            change_page(Page::Settings);
            break;
    }
}