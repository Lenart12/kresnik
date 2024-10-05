#pragma once

#define TFT_BUDERUS_BLUE 0x02F4

#define CONCAT_NAME(a, b, c) a##_##b##_##c
#define GUI_METHOD_(page_name, type) \
    HeatingController::CONCAT_NAME(gui, page_name, type)
#define GUI_METHOD(type) GUI_METHOD_(PAGE_NAME, type)

#include "Ubuntu_Regular18.h"
