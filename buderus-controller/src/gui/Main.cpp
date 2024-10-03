#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME Main

uint16_t main_temp_x[3] = { 0, 0, 0 };
uint16_t main_temp_y = 0;
uint16_t main_relay_y = 0;

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);

    _tft.setTextSize(3);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString("Buderus krmilnik", _tft.width() / 2, 10);
    _tft.setTextDatum(BL_DATUM);
    _tft.setTextSize(2);

    uint16_t x = 10;
    uint16_t y = _tft.height() -10;
    _tft.setTextColor(TFT_BLACK, TFT_BUDERUS_BLUE);
    x += _tft.drawString("Nastavitve ", x, y);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    x += _tft.drawString("Status ", x, y);
    _tft.setTextColor(TFT_YELLOW, TFT_BUDERUS_BLUE);
    _tft.drawString("Info", x, y);

    main_temp_x[0] = _tft.width() / 12 * 2;
    main_temp_x[1] = _tft.width() / 12 * 6;
    main_temp_x[2] = _tft.width() / 12 * 10;

    _tft.setTextSize(2);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.setTextDatum(BC_DATUM);
    y = _tft.height() / 2 - 50;

    _tft.drawString("Zunaj", main_temp_x[0], y);
    _tft.drawString("Hranilnik", main_temp_x[1], y);
    _tft.drawString("Pec", main_temp_x[2], y);

    main_temp_y = _tft.height() / 2;

    _tft.setTextSize(1);
    main_relay_y = _tft.height() / 2 + 75;
    y = main_relay_y - 15;
    _tft.drawString("Gorilec", main_temp_x[0], y);
    _tft.drawString("Hranilnik cirk.", main_temp_x[1], y);
    _tft.drawString("Gretje crik.", main_temp_x[2], y);

    _tft.fillSmoothCircle(main_temp_x[0], main_temp_y, 40, TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.fillSmoothCircle(main_temp_x[1], main_temp_y, 40, TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.fillSmoothCircle(main_temp_x[2], main_temp_y, 40, TFT_WHITE, TFT_BUDERUS_BLUE);

    #ifdef SIMULATE_TEMPS
    _tft.setTextColor(TFT_RED, TFT_BUDERUS_BLUE);
    _tft.setTextSize(3);
    _tft.drawString("SIMULACIJA", _tft.width() / 2, 30);
    #endif

    _tft.setTextDatum(MC_DATUM);
    _tft.setTextSize(2);
    _tft.setTextColor(TFT_BLACK, TFT_WHITE);
}

void GUI_METHOD(update)() {
    if (run_every(1000)) return;

    const uint8_t sensor_ids[] = { _sensor_outside_id, _sensor_dhw_storage_id, _sensor_furnace_id };
    char buffer[10];

    for (int i = 0; i < 3; i++) {
        float temp = get_temp(sensor_ids[i]);
        _tft.fillRect(main_temp_x[i] - 35, main_temp_y - 8, 70, 16, TFT_WHITE);
        if (isnan(temp)) {
            _tft.setTextColor(TFT_RED, TFT_WHITE);
            _tft.drawString("ERR", main_temp_x[i], main_temp_y);
        } else {
            _tft.setTextColor(TFT_BLACK, TFT_WHITE);
            _tft.drawFloat(temp, 1, main_temp_x[i], main_temp_y);
        }
    }

    const uint16_t COL_BORDER = TFT_BUDERUS_BLUE;
    const uint16_t COL_OFF = 0x9882;
    const uint16_t COL_ON = 0xf801;

    _tft.fillSmoothCircle(main_temp_x[0], main_relay_y, 10, _heating_active ? COL_ON : COL_OFF, COL_BORDER);
    _tft.fillSmoothCircle(main_temp_x[1], main_relay_y, 10, _storage_pump_active ? COL_ON : COL_OFF, COL_BORDER);
    _tft.fillSmoothCircle(main_temp_x[2], main_relay_y, 10, _heating_circulation_active ? COL_ON : COL_OFF, COL_BORDER);

}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {
    if (y < 210) return;
    if (x > 280) return;
    if (x > 225) return change_page(Page::Info);
    if (x > 133) return change_page(Page::Status);
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