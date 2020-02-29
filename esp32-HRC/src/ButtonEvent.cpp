#include <ButtonEvent.h>

uint8_t ButtonEvent::pressed_queue = 0x00;

uint32_t ButtonEvent::last_pressed_0 = 0;
uint32_t ButtonEvent::last_pressed_1 = 0;
uint32_t ButtonEvent::last_pressed_2 = 0;
uint32_t ButtonEvent::last_pressed_3 = 0;