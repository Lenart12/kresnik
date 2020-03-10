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

enum MotorDirection : uint8_t{
    dir_close = 0,
    dir_open = 1
};

void move_motor(uint8_t motor, MotorDirection direction, uint32_t time_ms);
void stop_motor(uint8_t motor);

bool relayRead(uint8_t pin);
void relayWrite(uint8_t pin, bool value);

struct Control{
    bool burner_enabled;
    float boiler_max_temp;
    float boiler_min_temp;

    bool hot_water_pump_enabled;
    float hot_water_min_temp;
    float hot_water_max_temp;

    bool circulator_pump_enabled;
    uint16_t circulator_duration;
    uint16_t circulator_start_time[8];

    bool underfloor_pump_enabled[4];
    float underfloor_wanted_temp[4];
    float tempature_slope;
    float night_time_correction;
    uint16_t night_time_start;
    uint16_t night_time_end;

    bool solar_pump_enabled;
    float max_solar_temp;
    float collector_temp_difference;
    bool inter_tank_pump_enabled;
    float inter_tank_temp_difference;
    float inter_tank_trigger_tempature; // ----------------------------------------------------
    bool vacation;

    TempatureAddress boiler_addr;
    TempatureAddress hot_water_container_addr;
    TempatureAddress enviroment_addr;
    TempatureAddress underfloor_addr[4];
    TempatureAddress solar_collector_addr;
    TempatureAddress solar_tank_addr;
    TempatureAddress heat_exchanger_addr;

    void setup();
    void update();
    void update_pumps();
    void load_default();
};

#endif