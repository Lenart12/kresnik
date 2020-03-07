#ifndef TEMPATURESELECT_H
#define TEMPATURESELECT_H

#include <Arduino.h>

#include <DallasTemperature.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <ButtonEvent.h>


struct SensorID{
    uint8_t oneWire_wire;
    DeviceAddress address;
};

class TempatureSelect{
public:
    TempatureSelect(Adafruit_ILI9341 *display,
	XPT2046_Touchscreen *ts,
	ButtonEvent *buttons,
    DallasTemperature *tempature_1,
    DallasTemperature *tempature_2) : 
    _display(display),
    _ts(ts),
    _buttons(_buttons),
    _tempature_1(tempature_1),
    _tempature_2(tempature_2)
    {};

    SensorID input();
private:
    Adafruit_ILI9341 *_display;
	XPT2046_Touchscreen *_ts;
	ButtonEvent *_buttons;
    DallasTemperature *_tempature_1;
    DallasTemperature *_tempature_2;
};

#endif