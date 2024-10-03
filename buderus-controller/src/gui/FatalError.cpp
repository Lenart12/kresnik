#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME FatalError

int seconds_to_restart = 600;

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_RED);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.setTextSize(3);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString("Kriticna napaka", _tft.width() / 2, 10);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextSize(1);
    _tft.drawString(_fatal_error_msg, _tft.width() / 2, _tft.height() / 2);
}

bool error_led = true;

void GUI_METHOD(update)() {
    if (run_every(1000)) return;
    for (int i = 0; i < 8; i++) {
        _pcf_led.digitalWrite(i, error_led);
    }
    error_led = !error_led;
    char buffer[30];
    sprintf(buffer, " Ponovni zagon cez %ds ", seconds_to_restart--);
    _tft.setTextDatum(BC_DATUM);
    _tft.setTextSize(2);
    _tft.drawString(buffer, _tft.width() / 2, _tft.height() - 10);
    _tft.setTextSize(1);
    _tft.drawString("Pritisni gumb za takojsen restart", _tft.width() / 2, _tft.height() - 40);
    if (seconds_to_restart < 0) {
        ESP.restart();
    }
}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {}

void GUI_METHOD(on_btn_press)(Button btn) {
    ESP.restart();
}