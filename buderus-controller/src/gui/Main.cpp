#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME Main

uint16_t main_temp_x[3] = { 0, 0, 0 };
uint16_t main_temp_y = 0;
uint16_t main_relay_y = 0;

uint16_t main_button_x[3] = { 0, 0, 0 };

void draw_temp_gauge(TFT_eSprite& _sprite, uint16_t x, uint16_t y, uint16_t r, float temp, float min_temp, float max_temp, float target_min, float target_max) {
    int32_t cx = _sprite.width() / 2;
    int32_t cy = _sprite.height() / 2;

    if (isnan(temp)) {
        _sprite.fillSprite(TFT_BUDERUS_BLUE);
        _sprite.fillSmoothCircle(cx, cy, r - 1, TFT_RED, TFT_BUDERUS_BLUE);
        _sprite.fillSmoothCircle(cx, cy, r - 6, TFT_WHITE, TFT_RED);
        _sprite.setTextColor(TFT_RED, TFT_WHITE);
        _sprite.drawString("ERR", cx, cy);
        _sprite.pushSprite(x - cx, y - cy);
        return;
    }

    const float arc_start = 60;
    const float arc_end = 300;
    const float arc_range = arc_end - arc_start;
    const float arc_target_min = std::max(arc_start + arc_range * (target_min - min_temp) / (max_temp - min_temp), arc_start);
    const float arc_target_max = std::min(arc_start + arc_range * (target_max - min_temp) / (max_temp - min_temp), arc_end);

    _sprite.fillSprite(TFT_BUDERUS_BLUE);
    _sprite.fillSmoothCircle(cx, cy, r, TFT_WHITE);
    _sprite.drawSmoothArc(cx, cy, r - 1, r - 6, arc_start, arc_end, TFT_BUDERUS_BLUE, TFT_WHITE);

    if (abs(target_min - target_max) > 0.1)
        _sprite.drawSmoothArc(cx, cy, r - 1, r - 6, arc_target_min, arc_target_max, TFT_RED, TFT_WHITE);

    float arc_temp = arc_start + arc_range * (temp - min_temp) / (max_temp - min_temp);
    arc_temp = min(arc_temp, arc_end);
    arc_temp = max(arc_temp, arc_start);

    // Draw the needle
    const float math_arc = arc_temp + 90;
    const float temp_cos = cos(math_arc * PI / 180);
    const float temp_sin = sin(math_arc * PI / 180);
    _sprite.drawWedgeLine(cx + temp_cos * (r - 14), cy + temp_sin * (r - 14), cx + temp_cos * (r - 7), cy + temp_sin * (r - 7), 3, 1, TFT_RED, TFT_WHITE);

    char buffer[10];
    sprintf(buffer, "%.1f", temp);
    _sprite.setTextColor(TFT_BLACK, TFT_WHITE);
    _sprite.drawString(buffer, cx, cy);

    _sprite.pushSprite(x - cx, y - cy);
}

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);

    _tft.setTextSize(3);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString("Buderus krmilnik", _tft.width() / 2, 20);
    _tft.setTextDatum(BL_DATUM);
    _tft.setTextSize(2);

    uint16_t x = 10;
    uint16_t y = _tft.height() -10;
    _tft.setTextColor(TFT_BLACK, TFT_BUDERUS_BLUE);
    x += _tft.drawString("Nastavitve ", x, y);
    main_button_x[0] = x;
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    x += _tft.drawString(" Status ", x, y);
    main_button_x[1] = x;
    _tft.setTextColor(TFT_YELLOW, TFT_BUDERUS_BLUE);
    x += _tft.drawString(" Info", x, y);
    main_button_x[2] = x;

    main_temp_x[0] = _tft.width() / 12 * 2;
    main_temp_x[1] = _tft.width() / 12 * 6;
    main_temp_x[2] = _tft.width() / 12 * 10;

    _tft.setTextSize(2);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.setTextDatum(BC_DATUM);
    y = _tft.height() / 2 - 40;

    _tft.drawString("Zunaj", main_temp_x[0], y);
    _tft.drawString("Hranilnik", main_temp_x[1], y);
    _tft.drawString("Peč", main_temp_x[2], y);

    main_temp_y = _tft.height() / 2;

    _tft.setTextSize(1);
    main_relay_y = _tft.height() / 2 + 75;
    y = main_relay_y - 15;
    _tft.drawString("Gorilnik", main_temp_x[0], y);
    _tft.drawString("Črp. hranilnik", main_temp_x[1], y);
    _tft.drawString("Črp. gretje", main_temp_x[2], y);

    #ifdef SIMULATE_TEMPS
    _tft.setTextColor(TFT_RED, TFT_BUDERUS_BLUE);
    _tft.setTextSize(3);
    _tft.drawString("SIMULACIJA", _tft.width() / 2, 60);
    #endif

    _sprite.createSprite(82, 82);
    _sprite.setTextDatum(MC_DATUM);
    _tft.setTextDatum(MC_DATUM);
}



void GUI_METHOD(update)() {
    if (run_every(1000)) return;

    const uint8_t sensor_ids[] = { _sensor_outside_id, _sensor_dhw_storage_id, _sensor_furnace_id };

    draw_temp_gauge(_sprite, main_temp_x[0], main_temp_y, 40, get_temp(sensor_ids[0]), -40, 40, 0, 40);
    draw_temp_gauge(_sprite, main_temp_x[1], main_temp_y, 40, get_temp(sensor_ids[1]), 10, 70, _dhw_storage_min_temp, _dhw_storage_max_temp);
    draw_temp_gauge(_sprite, main_temp_x[2], main_temp_y, 40, get_temp(sensor_ids[2]), 10, 70, _furnace_heater_target_min, _furnace_heater_target_max);

    const uint16_t COL_BORDER = TFT_BUDERUS_BLUE;
    const uint16_t COL_OFF = 0x9882;
    const uint16_t COL_ON = 0xf801;

    _tft.fillSmoothCircle(main_temp_x[0], main_relay_y, 10, _heating_active ? COL_ON : COL_OFF, COL_BORDER);
    _tft.fillSmoothCircle(main_temp_x[1], main_relay_y, 10, _storage_pump_active ? COL_ON : COL_OFF, COL_BORDER);
    _tft.fillSmoothCircle(main_temp_x[2], main_relay_y, 10, _heating_circulation_active ? COL_ON : COL_OFF, COL_BORDER);

}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {
    if (y < 210) return;
    if (x > main_button_x[2]) return;
    if (x > main_button_x[1]) return change_page(Page::Info);
    if (x > main_button_x[0]) return change_page(Page::Status);
    change_page(Page::Settings);
}

void GUI_METHOD(on_btn_press)(Button btn) {
    switch (btn) {
        case Button::Black:
            change_page(Page::Settings);
            break;
        case Button::White:
            change_page(Page::Status);
            break;
        case Button::Yellow:
            change_page(Page::Info);
            break;
    }
}