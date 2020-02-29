#ifndef UTIL_H
#define UTIL_H
#include <Arduino.h>
#include <Adafruit_ILI9341.h>

enum class Align : uint8_t{
	center = 0,
	left = 1,
	top = 1,
	right = 2,
	bottom = 2
};

void printAlignText(Adafruit_ILI9341 &display, String str, Align halign, Align valign, int16_t x, int16_t y, uint16_t w, uint16_t h);

#endif