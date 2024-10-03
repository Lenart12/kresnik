#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME Info

const char* info_text[] = {
    "Buderus krmilnik",
    "",
    "Verzija: " BK_FW_VERSION_STR,
    "Avtor: Lenart Kos",
    "Kontakt: ",
    "    koslenart@gmail.com",
    "",
    "Begunje pri Cerknici 2024",
    "",
    "        https://github.com/Lenart12/kresnik",
};

void GUI_METHOD(draw)() {
    _tft.fillScreen(TFT_BUDERUS_BLUE);
    _tft.setTextColor(TFT_WHITE, TFT_BUDERUS_BLUE);

    _tft.setTextSize(2);
    _tft.setTextDatum(TL_DATUM);
    uint16_t y = 10;
    for (int i = 0; i < sizeof(info_text) / sizeof(info_text[0]); i++) {
        if (i == 8) _tft.setTextSize(1);
        _tft.drawString(info_text[i], 10, y);
        y += _tft.fontHeight() + 5;
    }

    _tft.setTextSize(3);
    _tft.setTextDatum(BL_DATUM);
    uint16_t x = 5;
    _tft.setTextColor(TFT_BLACK);
    x += _tft.drawString("< ", x, _tft.height() - 5);
}

void GUI_METHOD(update)() {}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {
    if (x < 50 && y > _tft.height() - 50)
        change_page(Page::Main);
}

void GUI_METHOD(on_btn_press)(Button btn) {
    switch (btn) {
        case Button::Black:
            change_page(Page::Main);
            break;
    }
}