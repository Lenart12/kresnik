#if 0

#include "HeatingController.hpp"
#include "gui_helper.hpp"

#define PAGE_NAME ...

void GUI_METHOD(draw)() {}

void GUI_METHOD(update)() {}

void GUI_METHOD(on_touch)(uint16_t x, uint16_t y) {}

void GUI_METHOD(on_btn_press)(Button btn) {}

#endif

// ##### PAGE STUBS #####
#define TODO_PAGE_STUB(name) \
    void HeatingController::gui_##name##_draw() { _tft.fillScreen(TFT_RED); _ofr_drawString("TODO: " #name, 10, 10); } \
    void HeatingController::gui_##name##_update() {} \
    void HeatingController::gui_##name##_on_touch(uint16_t x, uint16_t y) {} \
    void HeatingController::gui_##name##_on_btn_press(Button btn) {}

// TODO_PAGE_STUB(Main)
