#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

#include <defines.h>
#include <control.h>

struct Settings{
    char wifi_ssid[32];
    char wifi_pass[32];
    Control control;
};

#endif
