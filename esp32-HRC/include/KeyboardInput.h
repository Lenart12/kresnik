#ifndef KEYBOARDINPUT_H
#define KEYBOARDINPUT_H

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <ButtonEvent.h>

#include <defines.h>

struct KeyboardButton{
	const uint16_t x, y;
	const char c;
	const char c_shift;
	const uint8_t w, h;
	KeyboardButton(uint16_t x, uint16_t y, char c, char c_shift) : x(x), y(y), c(c), c_shift(c_shift), w(20), h(35){};
	KeyboardButton(uint16_t x, uint16_t y, char c, char c_shift, uint8_t w, uint8_t h) : x(x), y(y), c(c), c_shift(c_shift), w(w), h(h){};
};

class KeyboardInput{
public:
	KeyboardInput(Adafruit_ILI9341 *display, XPT2046_Touchscreen *ts, ButtonEvent *buttons) :
	_display(display), _ts(ts), _buttons(buttons), _input(""){ }

	String input(const char* placeholder = "");

	void draw();
	bool update(bool &done);

private:
	Adafruit_ILI9341 *_display;
	XPT2046_Touchscreen *_ts;
	ButtonEvent *_buttons;

	String _input;
	String _placeholder;
	bool _shift;

	static const KeyboardButton _keyboard[51];
};

#endif