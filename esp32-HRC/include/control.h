#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>

#include <DallasTemperature.h>

class MotorControl{
public:
    MotorControl(){};

    void update();
private:
    float target_temp;
    DeviceAddress ds18b20_address;

};


enum class RelayControlType : uint8_t{
    timer,
    tempature,
    wifi
};

class RelayControl{
public:
    RelayControl(){};

    void update();
private:
    uint8_t gpio_number;
    RelayControlType type;

    // timer
    uint32_t timer_turn_on;
    uint32_t timer_turn_off;

    // tempature
    DeviceAddress ds18b20_address;
    float switch_tempature;
    bool inverted;
};

class Control{
public:
    Control(){};

    void update();
private:
    MotorControl motors[4];
    RelayControl relays[16];
};

#endif