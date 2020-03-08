#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include <Ticker.h>
#include <DallasTemperature.h>

struct TempatureAddress{
    uint8_t lane;
    DeviceAddress address;

    float read();
};

class MotorControl{
public:
    MotorControl():MotorControl(false){};
    MotorControl(bool set_up):
    set_up(false){};

    MotorControl(
        uint8_t id,
        bool enabled,
        float target_temp,
        float heating_offset_coeficient,
        TempatureAddress circuit_addr,
        TempatureAddress outside_addr):
        id(id),
        set_up(true),
        target_temp(target_temp),
        heating_offset_coeficient(heating_offset_coeficient),
        circuit_addr(circuit_addr),
        outside_addr(outside_addr)    
    {};

    void update();
protected:
    uint8_t id;

    bool set_up;
    bool enabled;

    float target_temp;
    float heating_offset_coeficient;

    TempatureAddress circuit_addr;
    TempatureAddress outside_addr;

    Ticker ticker;
};


enum class RelayControlType : uint8_t{
    time_of_day,
    tempature,
    wifi
};

class RelayControl{
public:
    RelayControl():
    set_up(false){};

    void update();
protected:
    uint8_t id;
    
    bool set_up;
    bool enabled;

    RelayControlType type;

    bool current_state;

    // time_of_day
    uint8_t days;
    uint32_t turn_on_seconds;
    uint32_t turn_off_seconds;

    // tempature
    TempatureAddress temp_addr;
    float switch_tempature;
    bool inverted;
};

/*

*/

class Control{
public:
    Control(){};

    void update();
private:
    MotorControl motors[4];
    RelayControl relays[16];
};

#endif