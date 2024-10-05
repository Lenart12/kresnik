#include "HeatingController.hpp"
#include "gui_helper.hpp"
#include  <functional>

#define PAGE_NAME Settings

struct SettingsLine {
    const char *name;
    std::function<void(char*)> current_value;
    std::function<void(SettingsLine&)> on_select;
};

const int settings_count = 11;
SettingsLine settings[settings_count];

int active_setting = 0;
int setting_offset = 0;
int max_settings_on_screen = 0;

void draw_setting_line(TFT_eSPI& _tft, SettingsLine& setting, uint16_t y, uint16_t fg, uint16_t bg) {
    _tft.fillRect(8, y - 2, _tft.width() - 16, _tft.fontHeight() + 4, bg);
    _tft.setTextColor(fg, bg);
    _tft.setTextDatum(TL_DATUM);
    _tft.drawString(setting.name, 10, y);
    char buffer[20];
    setting.current_value(buffer);
    _tft.setTextColor(TFT_LIGHTGREY, bg);
    _tft.setTextDatum(TR_DATUM);
    _tft.drawString(buffer, _tft.width() - 10, y);    
}

void draw_setting_labels(TFT_eSPI &_tft) {
    _tft.setTextDatum(TL_DATUM);

    char buffer[20];

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
        draw_setting_line(_tft, settings[i], y, fg, bg);
        y += _tft.fontHeight() + 5;
    }

    _tft.setTextDatum(BR_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE, true);
    char _buffer[10];
    sprintf(_buffer, "  %d/%d", active_setting + 1, settings_count);
    _tft.drawString(_buffer, _tft.width() - 35, _tft.height() - 5);
}

void swap_active_setting(TFT_eSPI &_tft, int old_setting, int new_setting) {
    char buffer[20];

    // Draw old setting, now white on blue
    uint16_t old_y = 40 + (old_setting - setting_offset) * (_tft.fontHeight() + 5);
    draw_setting_line(_tft, settings[old_setting], old_y, TFT_WHITE, TFT_BUDERUS_BLUE);


    // Draw new setting, now blue on white
    uint16_t new_y = 40 + (new_setting - setting_offset) * (_tft.fontHeight() + 5);
    draw_setting_line(_tft, settings[new_setting], new_y, TFT_BUDERUS_BLUE, TFT_WHITE);

    _tft.setTextDatum(BR_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE, true);
    char _buffer[10];
    sprintf(_buffer, "  %d/%d", active_setting + 1, settings_count);
    _tft.drawString(_buffer, _tft.width() - 35, _tft.height() - 5);
}

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString("Nastavitve", _tft.width() / 2, 10);
    _tft.setTextDatum(TL_DATUM);

    max_settings_on_screen = (_tft.height() - 60) / (_tft.fontHeight() + 5);

    _tft.setTextDatum(BL_DATUM);
    _tft.setTextColor(TFT_BLACK, TFT_BUDERUS_BLUE);
    _tft.drawString("< ", 5, _tft.height() - 5);
    _tft.setTextDatum(TR_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.drawString("^ ", _tft.width() - 5, 20);
    _tft.setTextDatum(BR_DATUM);
    _tft.setTextColor(TFT_YELLOW, TFT_BUDERUS_BLUE);
    _tft.drawString("v ", _tft.width() - 5, _tft.height() - 10);

    #define SETTINGS_LINE(setting_name, fmt, val) \
        settings[i].name = setting_name; \
        settings[i].current_value = [=](char* buffer) { sprintf(buffer, fmt, val); }; \
        settings[i++].on_select = [=](SettingsLine& setting)

    int i = 0;

    SETTINGS_LINE("Krmiljenje", "%s", _system_enabled ? "Vklopljeno" : "Izklopljeno") {
        _set_bool.name = setting.name;
        _set_bool.setting = &_system_enabled;
        change_page(Page::SetBool);
    };

    SETTINGS_LINE("Peč pri -15°C", "%.1f°C", _furnace_temp_n15) {
        _set_temp.name = setting.name;
        _set_temp.setting = &_furnace_temp_n15;
        _set_temp.min = 10;
        _set_temp.max = 70;
        change_page(Page::SetTempDegrees);
    };

    SETTINGS_LINE("Peč pri 15C°", "%.1f°C", _furnace_temp_p15) {
        _set_temp.name = setting.name;
        _set_temp.setting = &_furnace_temp_p15;
        _set_temp.min = 10;
        _set_temp.max = 70;
        change_page(Page::SetTempDegrees);
    };

    SETTINGS_LINE("Peč max. odstopanje", "%.1f°C", _furnace_max_off) {
        _set_temp.name = setting.name;
        _set_temp.setting = &_furnace_max_off;
        _set_temp.min = 3;
        _set_temp.max = 30;
        change_page(Page::SetTempDegrees);
    };

    SETTINGS_LINE("Hranilnik min. temp.", "%.1f°C", _dhw_storage_min_temp) {
        _set_temp.name = setting.name;
        _set_temp.setting = &_dhw_storage_min_temp;
        _set_temp.min = 10;
        _set_temp.max = _dhw_storage_max_temp;
        change_page(Page::SetTempDegrees);
    };

    SETTINGS_LINE("Hranilnik max. temp.", "%.1f°C", _dhw_storage_max_temp) {
        _set_temp.name = setting.name;
        _set_temp.setting = &_dhw_storage_max_temp;
        _set_temp.min = _dhw_storage_min_temp;
        _set_temp.max = 70;
        change_page(Page::SetTempDegrees);
    };

    SETTINGS_LINE("Cirkulacija gretja", "%s", _heating_circulation_enabled ? "Vklopljeno" : "Izklopljeno") {
        _set_bool.name = setting.name;
        _set_bool.setting = &_heating_circulation_enabled;
        change_page(Page::SetBool);
    };

    SETTINGS_LINE("Senzor zunaj", "%d", _sensor_outside_id) {
        _set_sensor.name = setting.name;
        _set_sensor.setting = &_sensor_outside_id;
        change_page(Page::SetTempSensor);
    };

    SETTINGS_LINE("Senzor peč", "%d", _sensor_furnace_id) {
        _set_sensor.name = setting.name;
        _set_sensor.setting = &_sensor_furnace_id;
        change_page(Page::SetTempSensor);
    };

    SETTINGS_LINE("Senzor hranilnik", "%d", _sensor_dhw_storage_id) {
        _set_sensor.name = setting.name;
        _set_sensor.setting = &_sensor_dhw_storage_id;
        change_page(Page::SetTempSensor);
    };

    SETTINGS_LINE("Avto. izklop zaslona", "%s", _screen_turn_off_enabled ? "Vklopljeno" : "Izklopljeno") {
        _set_bool.name = setting.name;
        _set_bool.setting = &_screen_turn_off_enabled;
        change_page(Page::SetBool);
    };

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
        if (new_setting >= settings_count) return;
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
                active_setting = settings_count - 1;
                setting_offset = max(0, settings_count - max_settings_on_screen);
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
            if (active_setting >= settings_count) {
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
            settings[active_setting].on_select(settings[active_setting]);
            break;
    }
}