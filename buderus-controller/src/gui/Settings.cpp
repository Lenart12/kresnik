#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME Settings

const char *setting_labels[] = {
    "Krmiljenje",
    "Pec pri -15C",
    "Pec pri 15C",
    "Pec max. odstop.",
    "Hranilnik min temp.",
    "Hranilnik max temp.",
    "Gretje cirkulacija",
    "Senzor zunaj",
    "Senzor pec",
    "Senzor hranilnik",
    "Avto. izklop zaslona",
};

int active_setting = 0;
int setting_offset = 0;
int max_settings_on_screen = 0;
const int setting_count = sizeof(setting_labels) / sizeof(setting_labels[0]);

void draw_setting_labels(TFT_eSPI &_tft) {
    _tft.setTextSize(2);
    _tft.setTextDatum(TL_DATUM);

    uint16_t y = 40;
    for (int i = setting_offset; i < setting_offset + max_settings_on_screen; i++) {
        uint16_t fg, bg;
        if (i == active_setting) {
            fg = TFT_BUDERUS_BLUE;
            bg = TFT_WHITE;
        } else {
            fg = TFT_WHITE;
            bg = TFT_BUDERUS_BLUE;
        }
        _tft.fillRect(8, y - 2, _tft.width() - 16, _tft.fontHeight() + 4, bg);
        _tft.setTextColor(fg);
        _tft.drawString(setting_labels[i], 10, y);
        y += _tft.fontHeight() + 5;
    }

    _tft.setTextSize(2);
    _tft.setTextDatum(BR_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    char _buffer[10];
    sprintf(_buffer, " %d/%d", active_setting + 1, setting_count);
    _tft.drawString(_buffer, _tft.width() - 35, _tft.height() - 5);
}

void swap_active_setting(TFT_eSPI &_tft, int old_setting, int new_setting) {
    _tft.setTextSize(2);
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.fillRect(8, 40 + (old_setting - setting_offset) * (_tft.fontHeight() + 5) - 2, _tft.width() - 16, _tft.fontHeight() + 4, TFT_BUDERUS_BLUE);
    _tft.drawString(setting_labels[old_setting], 10, 40 + (old_setting - setting_offset) * (_tft.fontHeight() + 5));
    _tft.fillRect(8, 40 + (new_setting - setting_offset) * (_tft.fontHeight() + 5) - 2, _tft.width() - 16, _tft.fontHeight() + 4, TFT_WHITE);
    _tft.setTextColor(TFT_BUDERUS_BLUE, TFT_WHITE);
    _tft.drawString(setting_labels[new_setting], 10, 40 + (new_setting - setting_offset) * (_tft.fontHeight() + 5));


    _tft.setTextSize(2);
    _tft.setTextDatum(BR_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    char _buffer[10];
    sprintf(_buffer, " %d/%d", active_setting + 1, setting_count);
    _tft.drawString(_buffer, _tft.width() - 35, _tft.height() - 5);
}

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE);
    _tft.setTextSize(3);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString("Nastavitve", _tft.width() / 2, 10);
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextSize(2);

    uint16_t max_height = _tft.height() - _tft.fontHeight() - 10;
    max_settings_on_screen = (_tft.height() - 60) / (_tft.fontHeight() + 5);

    _tft.setTextDatum(BL_DATUM);
    _tft.setTextColor(TFT_BLACK);
    _tft.drawString("< ", 5, _tft.height() - 5);
    _tft.setTextDatum(TR_DATUM);
    _tft.setTextColor(TFT_WHITE);
    _tft.drawString("^ ", _tft.width() - 5, 20);
    _tft.setTextDatum(BR_DATUM);
    _tft.setTextColor(TFT_YELLOW);
    _tft.drawString("v ", _tft.width() - 5, _tft.height() - 10);

    draw_setting_labels(_tft);
}

void GUI_METHOD(update)() {}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {
    if (y < 32) {
        if (x > _tft.width() - 50) return GUI_METHOD(on_btn_press)(Button::White);
        return;
    }

    if (y < _tft.height() - 32) {
        int old_setting = active_setting;
        y -= 40;
        int new_setting = y / (_tft.fontHeight() + 5) + setting_offset;
        if (new_setting >= setting_count) return;
        if (new_setting == active_setting) return GUI_METHOD(on_btn_press)(Button::Blue);
        active_setting = new_setting;
        swap_active_setting(_tft, old_setting, active_setting);
        return;
    }


    if (x < 50) return GUI_METHOD(on_btn_press)(Button::Black);
    if (x > _tft.width() - 50) return GUI_METHOD(on_btn_press)(Button::Yellow);
}

void GUI_METHOD(on_btn_press)(Button btn) {
    int old_setting = active_setting;
    int old_offset = setting_offset;
    switch (btn) {
        case Button::Black:
            save_settings();
            _valid_config = validate_config();
            _last_heating_update = 0;
            change_page(Page::Main);
            break;
        case Button::White:
            active_setting--;
            if (active_setting < 0) {
                active_setting = setting_count - 1;
                setting_offset = max(0, setting_count - max_settings_on_screen);
            } else if (active_setting < setting_offset) {
                setting_offset--;
            }
            if (old_offset != setting_offset)
                draw_setting_labels(_tft);
            else
                swap_active_setting(_tft, old_setting, active_setting);
            break;
        case Button::Yellow:
            active_setting++;
            if (active_setting >= setting_count) {
                active_setting = 0;
                setting_offset = 0;
            } else if (active_setting >= setting_offset + max_settings_on_screen) {
                setting_offset++;
            }
            if (old_offset != setting_offset)
                draw_setting_labels(_tft);
            else
                swap_active_setting(_tft, old_setting, active_setting);
            break;
        case Button::Blue:
            switch (active_setting) {
                case 0: // Krmiljenje
                    _set_bool.name = setting_labels[active_setting];
                    _set_bool.setting = &_system_enabled;
                    change_page(Page::SetBool);
                    break;
                case 1: // Pec pri -15C
                    _set_temp.name = setting_labels[active_setting];
                    _set_temp.setting = &_furnace_temp_n15;
                    _set_temp.min = 10;
                    _set_temp.max = 90;
                    change_page(Page::SetTempDegrees);
                    break;
                case 2: // Pec pri 15C
                    _set_temp.name = setting_labels[active_setting];
                    _set_temp.setting = &_furnace_temp_p15;
                    _set_temp.min = 10;
                    _set_temp.max = 90;
                    change_page(Page::SetTempDegrees);
                    break;
                case 3: // Pec max. odstop.
                    _set_temp.name = setting_labels[active_setting];
                    _set_temp.setting = &_furnace_max_off;
                    _set_temp.min = 5;
                    _set_temp.max = 30;
                    change_page(Page::SetTempDegrees);
                    break;
                case 4: // Hranilnik min temp.
                    _set_temp.name = setting_labels[active_setting];
                    _set_temp.setting = &_dhw_storage_min_temp;
                    _set_temp.min = 10;
                    _set_temp.max = _dhw_storage_max_temp;
                    change_page(Page::SetTempDegrees);
                    break;
                case 5: // Hranilnik max temp.
                    _set_temp.name = setting_labels[active_setting];
                    _set_temp.setting = &_dhw_storage_max_temp;
                    _set_temp.min = _dhw_storage_min_temp;
                    _set_temp.max = 90;
                    change_page(Page::SetTempDegrees);
                    break;
                case 6: // Gretje cirkulacija
                    _set_bool.name = setting_labels[active_setting];
                    _set_bool.setting = &_heating_circulation_enabled;
                    change_page(Page::SetBool);
                    break;
                case 7: // Senzor zunaj
                    _set_sensor.name = setting_labels[active_setting];
                    _set_sensor.setting = &_sensor_outside_id;
                    change_page(Page::SetTempSensor);
                    break;
                case 8: // Senzor pec
                    _set_sensor.name = setting_labels[active_setting];
                    _set_sensor.setting = &_sensor_furnace_id;
                    change_page(Page::SetTempSensor);
                    break;
                case 9: // Senzor hranilnik
                    _set_sensor.name = setting_labels[active_setting];
                    _set_sensor.setting = &_sensor_dhw_storage_id;
                    change_page(Page::SetTempSensor);
                    break;
                case 10: // Avto. izklop zaslona
                    _set_bool.name = setting_labels[active_setting];
                    _set_bool.setting = &_screen_turn_off_enabled;
                    change_page(Page::SetBool);
            }
            break;
    }
}