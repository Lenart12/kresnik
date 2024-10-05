#include "HeatingController.hpp"
#include "gui_helper.hpp"
#include <functional>

#define PAGE_NAME Status

struct StatusLine {
    const char* name;
    uint16_t name_width;
    std::function<void(char*)> get;
};

const int status_count = 12;
StatusLine statuses[status_count];

uint8_t status_offset = 0;
uint8_t max_status_per_screen = 0;

void draw_status_value(TFT_eSPI& tft) {
    int max_i = min(status_count, status_offset + max_status_per_screen);
    uint16_t y = 40;
    uint16_t x = tft.width() - 10;
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BUDERUS_BLUE);
    for (int i = status_offset; i < max_i; i++) {
        char buffer[20];
        statuses[i].get(buffer);
        tft.fillRect(statuses[i].name_width, y, tft.width() - statuses[i].name_width, tft.fontHeight() + 5, TFT_BUDERUS_BLUE);
        tft.drawString(buffer, x, y);
        y += tft.fontHeight() + 5;
    }
}

void draw_status_name(TFT_eSPI& tft) {
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    tft.setTextSize(2);
    uint16_t y = 40;
    int i = status_offset;
    for (i = status_offset; i < min(status_count, status_offset + max_status_per_screen); i++) {
        tft.fillRect(10, y, tft.width() - 20, tft.fontHeight() + 5, TFT_BUDERUS_BLUE);
        statuses[i].name_width = 10 + tft.drawString(statuses[i].name, 10, y);
        y += tft.fontHeight() + 5;
    }
    tft.setTextDatum(BR_DATUM);
    char buffer[10];
    sprintf(buffer, "  %d-%d/%d", status_offset + 1, i, status_count);
    tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE, true);
    tft.drawString(buffer, tft.width() - 35, tft.height() - 5);
}

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.setTextDatum(TC_DATUM);
    _tft.setTextSize(3);
    _tft.drawString("Status", _tft.width() / 2, 10);
    _tft.setTextSize(2);

    max_status_per_screen = (_tft.height() - 60) / (_tft.fontHeight() + 5);

    int i = 0;

    #define STATUS_LINE(status_name) statuses[i].name = status_name; statuses[i++].get = [=](char* buffer)

    STATUS_LINE("Krmilnik vklopljen") {
        sprintf(buffer, "%s", _system_enabled ? "DA" : "NE");
    };

    STATUS_LINE("Krmilnik zagnan") {
        sprintf(buffer, "%s", system_valid_for_control() ? "DA" : "NE");
    };

    STATUS_LINE("Veljavna konfiguracija") {
        sprintf(buffer, "%s", _valid_config ? "DA" : "NE");
    };

    STATUS_LINE("Temperatura zunaj") {
        float temp = get_temp(_sensor_outside_id);
        if (std::isnan(temp)) {
            sprintf(buffer, "Napaka");
        } else {
            sprintf(buffer, "%.1fC", temp);
        }
    };

    STATUS_LINE("Temperatura peči") {
        float temp = get_temp(_sensor_furnace_id);
        if (std::isnan(temp)) {
            sprintf(buffer, "Napaka");
        } else {
            sprintf(buffer, "%.1fC", temp);
        };
    };

    STATUS_LINE("Temperatura hranilnika") {
        float temp = get_temp(_sensor_dhw_storage_id);
        if (std::isnan(temp)) {
            sprintf(buffer, "Napaka");
        } else {
            sprintf(buffer, "%.1fC", temp);
        };
    };

    STATUS_LINE("Stanje") {
        switch (_heating_state) {
            case HeatingState::Init:
                sprintf(buffer, "Zagon");
                break;
            case HeatingState::Idle:
                sprintf(buffer, "Deluje");
                break;
            case HeatingState::HeatingFurnace:
                sprintf(buffer, "Gretje peči");
                break;
            case HeatingState::HeatingDhwStorage:
                sprintf(buffer, "Gretje hranilnika");
                break;
        }
    };

    STATUS_LINE("Gorilnik") {
        sprintf(buffer, "%s", _heating_active ? "Prižgan" : "Ugasnjen");
    };

    STATUS_LINE("Črpalka hranilnika") {
        sprintf(buffer, "%s", _storage_pump_active ? "Prižgana" : "Ugasnjena");
    };

    STATUS_LINE("Črpalka gretja") {
        sprintf(buffer, "%s", _heating_circulation_active ? "Prižgana" : "Ugasnjena");
    };

    STATUS_LINE("Ciljna temp. peči") {
        sprintf(buffer, "%.1fC-%.1fC", target_furnace_temp() - _furnace_max_off, target_furnace_temp());
    };

    STATUS_LINE("Ciljna temp. hranilnka") {
        sprintf(buffer, "%.1fC-%.1fC", _dhw_storage_min_temp, _dhw_storage_max_temp);
    };

    _tft.setTextDatum(BL_DATUM);
    _tft.setTextColor(TFT_BLACK, TFT_BUDERUS_BLUE);
    _tft.drawString("< ", 5, _tft.height() - 5);
    _tft.setTextDatum(TR_DATUM);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);
    _tft.drawString("^ ", _tft.width() - 5, 20);
    _tft.setTextDatum(BR_DATUM);
    _tft.setTextColor(TFT_YELLOW, TFT_BUDERUS_BLUE);
    _tft.drawString("v ", _tft.width() - 5, _tft.height() - 10);

    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);

    draw_status_name(_tft);
    draw_status_value(_tft);
}

void GUI_METHOD(update)() {
    if (run_every(1000)) return;
    draw_status_value(_tft);
}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {
    if (y < 32 && x > _tft.width() - 50) return GUI_METHOD(on_btn_press)(Button::White);
    if (y < _tft.height() - 32) return;
    if (x < 50) return change_page(Page::Main);
    if (x > _tft.width() - 50) return GUI_METHOD(on_btn_press)(Button::Yellow);
}

void GUI_METHOD(on_btn_press)(Button btn) {
    switch (btn) {
        case Button::Black:
            change_page(Page::Main);
            break;
        case Button::White:
            if (status_offset > 0) {
                status_offset--;
                draw_status_name(_tft);
                draw_status_value(_tft);
            }
            break;
        case Button::Yellow:
            if (status_offset + max_status_per_screen < status_count) {
                status_offset++;
                draw_status_name(_tft);
                draw_status_value(_tft);
            }
            break;
    }
}