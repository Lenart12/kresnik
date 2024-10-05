/*
 * File: HeatingController.hpp
 * Lenart (c) 2024 koslenart@gmail.com 
 */
#pragma once

#include "Wire.h"
#include "TFT_eSPI.h"
#include "TempController.hpp"
#include "PCF8574.h"

// #define SIMULATE_TEMPS

class HeatingController {
public:
    void run();
private:
    void setup_devices();

    void process_input();

    _Noreturn void fatal_error(const char* msg);
private:
    // ##### DEVICES #####

    TFT_eSPI _tft;
    TFT_eSprite _sprite {&_tft};

    enum class Button {
        Black,
        White,
        Yellow,
        Blue,
    };
    unsigned long _last_input = 0;
    bool _input_locked = false;

    TempController _temp_0 = TempController(DS2482_0);
    TempController _temp_1 = TempController(DS2482_1);

    PCF8574 _pcf_led {PCF8574_STATUS};
    PCF8574 _pcf_relay {PCF8574_RELAY0};

    enum class Relay {
        FurnaceHeater,
        DhwStoragePump,
        HeatingPump,
    };

    enum class StatusLed {
        PowerOn,
        SystemRunning,
        StateIdle,
        StateHeatingFurnace,
        StateHeatingDhwStorage,
        RelayFurnaceHeater,
        RelayDhwStoragePump,
        RelayHeatingPump,
    };

    void set_relay(Relay relay, bool state);
    void set_status_led(StatusLed led, bool state);
    float get_temp(uint8_t sensor_id);

    // ##### HEATING LOGIC #####

    enum class HeatingState {
        Init,
        Idle,
        HeatingFurnace,
        HeatingDhwStorage,
    } _heating_state = HeatingState::Init;

    bool _furnace_heater_ongoing = false;

    bool _heating_active = false;
    bool _storage_pump_active = false;
    bool _heating_circulation_active = false;

    float _furnace_heater_target_min = 0;
    float _furnace_heater_target_max = 0;

    void switch_state(HeatingState next);
    bool system_valid_for_control();
    void process_heating();
    float target_furnace_temp();

    unsigned long _last_heating_update = 0;

    #ifdef SIMULATE_TEMPS

    float _sim_temp_outside = 10;
    float _sim_temp_furnace = 50;
    float _sim_temp_dhw_storage = 40;

    void simulate_temps();
    unsigned long _last_sim_update = 0;

    #endif

    // ##### SETTINGS #####

    bool _system_enabled = false;
    float _furnace_temp_p15 = 50;
    float _furnace_temp_n15 = 70;
    float _furnace_max_off = 15;
    bool _heating_circulation_enabled = true;
    float _dhw_storage_min_temp = 40;
    float _dhw_storage_max_temp = 45;
    uint8_t _sensor_outside_id = 0;
    uint8_t _sensor_furnace_id = 1;
    uint8_t _sensor_dhw_storage_id = 2;

    void load_settings();
    void save_settings();

    bool _valid_config = false;
    bool validate_config();

    // ##### GUI #####

    #define HC_PAGES_FOR_EACH(XX) \
        XX(Main) \
        XX(Info) \
        XX(FatalError) \
        XX(Settings) \
            XX(SetTempSensor)  \
            XX(SetTempDegrees) \
            XX(SetBool) \
        XX(Status) \

    // Page type enum
    enum class Page {
        #define PAGES_ENUM(name) name,
        HC_PAGES_FOR_EACH(PAGES_ENUM)
        #undef PAGES_ENUM
        PageCount,
        RedrawPage,
        Initialize,
    } _current_page = Page::Settings;

    // Current page
    void (HeatingController::*_page_draw)() = nullptr;
    void (HeatingController::*_page_update)() = nullptr;
    void (HeatingController::*_on_touch)(uint16_t x, uint16_t y) = nullptr;
    void (HeatingController::*_on_btn_press)(Button btn) = nullptr;

    void change_page(Page page_type);

    // Page method declarations
    #define PAGES_DECLARE(name) \
        void gui_##name##_draw(); \
        void gui_##name##_update(); \
        void gui_##name##_on_touch(uint16_t x, uint16_t y); \
        void gui_##name##_on_btn_press(Button btn);
    HC_PAGES_FOR_EACH(PAGES_DECLARE)
    #undef PAGES_DECLARE

    // Update logic timeout
    bool run_every(unsigned long interval);
    unsigned long _gui_last_update = 0;

    // Screen turn off timeout
    bool _screen_on = true;
    bool _screen_turn_off_enabled = true;
    void (HeatingController::*_screen_on_on_touch)(uint16_t x, uint16_t y) = nullptr;
    void (HeatingController::*_screen_on_on_btn_press)(Button btn) = nullptr;
    void process_screen_timeout();
    void set_screen_enabled(bool enable);

    void screen_timeout_on_touch(uint16_t, uint16_t) {};
    void screen_timeout_on_btn_press(Button) {};

    // ##### PAGE PARAMS #####
    const char* _fatal_error_msg = "Kriticna napaka!";

    struct SetTempDegreesParams {
        const char* name = "";
        float* setting = nullptr;
        float min = 0;
        float max = 0;
    } _set_temp;

    struct SetTempSensorParams {
        const char* name = "";
        uint8_t* setting = nullptr;
    } _set_sensor;

    struct SetBool {
        const char* name = "";
        bool* setting = nullptr;
    } _set_bool;
};
