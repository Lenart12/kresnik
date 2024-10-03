/*
 * File: HeatingController.cpp
 * Lenart (c) 2024 koslenart@gmail.com 
 */


#include "defines.h"
#include "HeatingController.hpp"
#include "gui_helper.hpp"
#include "Preferences.h"

#ifdef SIMULATE_TEMPS
#include <stdlib.h>
#endif


void HeatingController::run() {
    load_settings();
    setup_devices();

    change_page(Page::Main);

    for (;;) {
        process_input();
        process_screen_timeout();
        _temp_0.Process();
        _temp_1.Process();

        #ifdef SIMULATE_TEMPS
        simulate_temps();
        #endif

        process_heating();
        if (_screen_on)
            (this->*_page_update)();
    }
}

void HeatingController::setup_devices() {
    // Serial
    Serial.begin(115200);

    // Setup TFT & Touch
    pinMode(TFT_LED_NEN, OUTPUT);
    digitalWrite(TFT_LED_NEN, LOW);

    _tft.init();
    _tft.setRotation(1);
    _tft.setTouch(TOUCH_CALIBRATION);
    _tft.fillScreen(TFT_BUDERUS_BLUE);

    // Buttons
    for (uint8_t i = 0; i < sizeof(BTN_PINS) / sizeof(BTN_PINS[0]); i++) {
        pinMode(BTN_PINS[i], INPUT);
    }

    // GPIO
    _pcf_led.begin();
    _pcf_relay.begin();

    for (int i = 0; i < 8; i++) {
        _pcf_led.pinMode(i, OUTPUT);
        _pcf_relay.pinMode(i, OUTPUT);
        _pcf_led.digitalWrite(i, HIGH);
        _pcf_relay.digitalWrite(i, HIGH);
    }

    // Setup temp sensors
    _temp_0.Process();
    _temp_1.Process();

    if (_temp_0.GetDeviceCount() + _temp_1.GetDeviceCount() != 3) {
        char buffer[50];
        sprintf(buffer, "Napaka pri inicializaciji senzorjev! %d+%d/3", _temp_0.GetDeviceCount(), _temp_1.GetDeviceCount());
        fatal_error(buffer);
    }

    #ifdef SIMULATE_TEMPS
    srand(millis());
    #endif

    // Finaly, turn on the power led
    set_status_led(StatusLed::PowerOn, true);
}

void HeatingController::process_input()
{
    if (_last_input != 0 && millis() - _last_input < 100) return;

    uint16_t x, y;
    if (_tft.getTouch(&x, &y)) {
        _last_input = millis();
        if (_input_locked) return;
        (this->*_on_touch)(x, y);
        _input_locked = true;
        return;
    }

    #define CHECK_BTN(pin, col) \
        if (digitalRead(BTN_##pin##_PIN) == LOW) { \
            _last_input = millis(); \
            if (_input_locked) return; \
            (this->*_on_btn_press)(Button::col); \
            _input_locked = true; \
            return; \
        }

    CHECK_BTN(B, Black)
    CHECK_BTN(W, White)
    CHECK_BTN(Y, Yellow)
    CHECK_BTN(C, Blue)

    _input_locked = false;
}

void HeatingController::fatal_error(const char *msg)
{
    set_screen_enabled(true);
    for (int i = 0; i < 8; i++) {
        _pcf_relay.digitalWrite(i, HIGH);
    }
    Serial.println(msg);
    _fatal_error_msg = msg;
    change_page(Page::FatalError);
    __unreachable();
}

void HeatingController::set_relay(Relay relay, bool state)
{
    switch (relay) {
        case Relay::FurnaceHeater:
            _heating_active = state;
            _pcf_relay.digitalWrite(0, !state);
            set_status_led(StatusLed::RelayFurnaceHeater, state);
            break;
        case Relay::DhwStoragePump:
            _storage_pump_active = state;
            _pcf_relay.digitalWrite(1, !state);
            set_status_led(StatusLed::RelayDhwStoragePump, state);
            break;
        case Relay::HeatingPump:
            _heating_circulation_active = state;
            _pcf_relay.digitalWrite(2, !state);
            set_status_led(StatusLed::RelayHeatingPump, state);
            break;
    }
    _gui_last_update = 0;
}

void HeatingController::set_status_led(StatusLed led, bool state)
{
    _pcf_led.digitalWrite(static_cast<uint8_t>(led), !state);

    // Special case for state leds, only one can be on at a time
    if (state == true) {
        switch (led)
        {
            case StatusLed::StateIdle:
                _pcf_led.digitalWrite(static_cast<uint8_t>(StatusLed::StateHeatingFurnace), HIGH);
                _pcf_led.digitalWrite(static_cast<uint8_t>(StatusLed::StateHeatingDhwStorage), HIGH);
                break;
            case StatusLed::StateHeatingFurnace:
                _pcf_led.digitalWrite(static_cast<uint8_t>(StatusLed::StateIdle), HIGH);
                _pcf_led.digitalWrite(static_cast<uint8_t>(StatusLed::StateHeatingDhwStorage), HIGH);
                break;
            case StatusLed::StateHeatingDhwStorage:
                _pcf_led.digitalWrite(static_cast<uint8_t>(StatusLed::StateIdle), HIGH);
                _pcf_led.digitalWrite(static_cast<uint8_t>(StatusLed::StateHeatingFurnace), HIGH);
                break;
        }
    }
}

float HeatingController::get_temp(uint8_t sensor_id)
{
    #ifndef SIMULATE_TEMPS
    if (sensor_id > 2) return NAN;
    if (sensor_id >= _temp_0.GetDeviceCount()) {
        if (_temp_1.GetLastReadMillis() == 0) return NAN;
        return _temp_1.GetTemperatureC(sensor_id - _temp_0.GetDeviceCount());
    }
    if (_temp_0.GetLastReadMillis() == 0) return NAN;
    return _temp_0.GetTemperatureC(sensor_id);
    #else
    if (sensor_id == _sensor_furnace_id) return _sim_temp_furnace;
    if (sensor_id == _sensor_dhw_storage_id) return _sim_temp_dhw_storage;
    if (sensor_id == _sensor_outside_id) return _sim_temp_outside;
    return NAN;
    #endif
}

#ifdef SIMULATE_TEMPS

void HeatingController::simulate_temps() {
    if (_last_sim_update != 0 && millis() - _last_sim_update < 1000) return;
    _last_sim_update = millis();

    const bool sim_runaway_furnace = false;

    // Outside not changing

    // Furnace temp cools down if not heating
    if (_heating_active || sim_runaway_furnace) {
        _sim_temp_furnace += 1 + rand() % 10 / 10.0;
    } else {
        _sim_temp_furnace -= rand() % 5 / 10.0;
    }

    // Storage temp cools down
    _sim_temp_dhw_storage -= rand() % 5 / 10.0;

    // Furnace cools down faster if circulation pump is on
    if (_heating_circulation_active) {
        _sim_temp_furnace -= rand() % 10 / 10.0;
    }

    // If dhw storage pump is on, storage temp and furnace temp will slowly equalize
    if (_storage_pump_active) {
        float dT = _sim_temp_furnace - _sim_temp_dhw_storage;
        if (dT > 0) {
            _sim_temp_furnace -= dT / 10;
            _sim_temp_dhw_storage += dT / 10;
        } else {
            _sim_temp_furnace += dT / 10;
            _sim_temp_dhw_storage -= dT / 10;
        }
    }
}

#endif

bool HeatingController::system_valid_for_control()
{
    if (!_system_enabled) return false;

    if (!_valid_config) return false;

    #ifndef SIMULATE_TEMPS
    unsigned long t0 = _temp_0.GetLastReadMillis();
    unsigned long t1 = _temp_1.GetLastReadMillis();
    unsigned long tn = millis();
    if (t0 == 0 || t1 == 0) return false;

    if (tn - t0 > 3000 || tn - t1 > 3000) return false;
    #endif

    if (isnan(get_temp(_sensor_outside_id))) return false;
    if (isnan(get_temp(_sensor_furnace_id))) return false;
    if (isnan(get_temp(_sensor_dhw_storage_id))) return false;

    return true;
}

void HeatingController::process_heating()
{
    // Run every 1s
    if (_last_heating_update != 0 && millis() - _last_heating_update < 1000) return;
    _last_heating_update = millis();

    if (!system_valid_for_control()) {
        if (_heating_state != HeatingState::Init) {
            set_status_led(StatusLed::SystemRunning, false);
            switch_state(HeatingState::Init);
        }
        return;
    }

    float outside_temp = get_temp(_sensor_outside_id);
    float furnace_temp = get_temp(_sensor_furnace_id);
    float dhw_storage_temp = get_temp(_sensor_dhw_storage_id);

    // Watchdog: if any of the sensors is reading too high, turn off everything
    if (outside_temp > 95 || furnace_temp > 95 || dhw_storage_temp > 95) {
        char buffer[50];
        sprintf(buffer, "Previsoka temperatura na senzorjih! %.1f %.1f %.1f", outside_temp, furnace_temp, dhw_storage_temp);
        fatal_error(buffer);
        return;
    }

    bool require_heating = false;
    bool require_dhw_storage_pump = false;
    bool require_heating_circulation_pump = false;

    switch (_heating_state) {
        case HeatingState::Init:
            set_status_led(StatusLed::SystemRunning, true);
            switch_state(HeatingState::Idle);
            break;
        case HeatingState::Idle:
            // Idle state, check if we need to heat something
            // Relays:
            //  - Furnace heater: OFF
            //  - DHW storage pump: OFF
            //  - Heating circulation pump: ON

            require_heating_circulation_pump = _heating_circulation_enabled;

            // 1. Check if DHW storage tank is too cold
            if (dhw_storage_temp < _dhw_storage_min_temp) {
                _furnace_heater_ongoing = furnace_temp < _dhw_storage_max_temp;
                require_dhw_storage_pump = furnace_temp > dhw_storage_temp;
                switch_state(HeatingState::HeatingDhwStorage);
                break;
            }
            // 2. Check if furance is too cold
            if (furnace_temp < target_furnace_temp() - _furnace_max_off) {
                switch_state(HeatingState::HeatingFurnace);
                break;
            }
            break;
        case HeatingState::HeatingFurnace:
            // Furnace is too cold, heat it until it reaches target temp
            // Relays:
            //  - Furnace heater: ON
            //  - DHW storage pump: OFF
            //  - Heating circulation pump: ON

            require_heating = true;
            require_heating_circulation_pump = _heating_circulation_enabled;

            // 1. Check if furnace is too hot
            if (furnace_temp >= target_furnace_temp()) {
                switch_state(HeatingState::Idle);
                break;
            }

            break;
        case HeatingState::HeatingDhwStorage:
            // DHW storage tank is too cold, heat it until it reaches _dhw_storage_max_temp
            // Heating is done by heating furnace tank until it reaches the interval [storage tank temp + 5C]
            // and then turning on the pump to circulate the water

            // Relays:
            //  - Furnace heater: when furnace temp is lower than storage temp
            //  - DHW storage pump: when furnace temp is atleast 1c larger than storage temp (ignore <1c differences)
            //  - Heating circulation pump: OFF

            // 1. Check if storage tank is too hot
            if (dhw_storage_temp >= _dhw_storage_max_temp) {
                switch_state(HeatingState::Idle);
                break;
            }

            // 2. Heat or cool furnace in interval [max(furnace temp, dhw_storage_temp + 5C)]
            if (_furnace_heater_ongoing) {
                if (furnace_temp >= max(target_furnace_temp(), _dhw_storage_max_temp + 5)) {
                    _furnace_heater_ongoing = false;
                }
            } else {
                if (furnace_temp < max(target_furnace_temp() - _furnace_max_off, _dhw_storage_max_temp)) {
                    _furnace_heater_ongoing = true;
                }
            }
            require_heating = _furnace_heater_ongoing;
            
            // 3. Turn on pump when furnace temp is larger than storage temp
            if (abs(furnace_temp - dhw_storage_temp) < 1)
                require_dhw_storage_pump = _storage_pump_active;
            else 
                require_dhw_storage_pump = furnace_temp >= dhw_storage_temp + 1;
            break;
    }

    if (require_heating != _heating_active) set_relay(Relay::FurnaceHeater, require_heating);
    if (require_dhw_storage_pump != _storage_pump_active) set_relay(Relay::DhwStoragePump, require_dhw_storage_pump);
    if (require_heating_circulation_pump != _heating_circulation_active) set_relay(Relay::HeatingPump, require_heating_circulation_pump);
}

void HeatingController::switch_state(HeatingState next)
{    
    _heating_state = next;
    switch (_heating_state)
    {
        case HeatingState::Init:
            set_relay(Relay::FurnaceHeater, false);
            set_relay(Relay::DhwStoragePump, false);
            set_relay(Relay::HeatingPump, false);
            set_status_led(StatusLed::StateIdle, false);
            set_status_led(StatusLed::StateHeatingFurnace, false);
            set_status_led(StatusLed::StateHeatingDhwStorage, false);
            break;
        case HeatingState::Idle:
            set_status_led(StatusLed::StateIdle, true);
            break;
        case HeatingState::HeatingFurnace:
            set_status_led(StatusLed::StateHeatingFurnace, true);
            break;
        case HeatingState::HeatingDhwStorage:
            set_status_led(StatusLed::StateHeatingDhwStorage, true);
            break;
    }
    _gui_last_update = 0;
}

float HeatingController::target_furnace_temp()
{
    // Interpolate between _furnace_temp_n15 and _furnace_temp_p15 for current outside temp
    float outside_temp = get_temp(_sensor_outside_id);

    float temp_diff = _furnace_temp_p15 - _furnace_temp_n15;
    float temp_diff_per_c = temp_diff / 30;

    return min(90.0f, _furnace_temp_n15 + (outside_temp + 15) * temp_diff_per_c);
}

void HeatingController::load_settings()
{
    Preferences prefs;
    prefs.begin("heating", false);

    _system_enabled = prefs.getBool("sys_en", _system_enabled);
    _furnace_temp_p15 = prefs.getFloat("ft_p15", _furnace_temp_p15);
    _furnace_temp_n15 = prefs.getFloat("ft_n15", _furnace_temp_n15);
    _furnace_max_off = prefs.getFloat("ft_off", _furnace_max_off);
    _heating_circulation_enabled = prefs.getBool("he_cir", _heating_circulation_enabled);
    _dhw_storage_min_temp = prefs.getFloat("dhw_min", _dhw_storage_min_temp);
    _dhw_storage_max_temp = prefs.getFloat("dhw_max", _dhw_storage_max_temp);
    _sensor_outside_id = prefs.getUChar("ts_out", _sensor_outside_id);
    _sensor_furnace_id = prefs.getUChar("ts_fur", _sensor_furnace_id);
    _sensor_dhw_storage_id = prefs.getUChar("ts_dhw", _sensor_dhw_storage_id);
    _screen_turn_off_enabled = prefs.getBool("scr_off", _screen_turn_off_enabled);

    prefs.end();

    _valid_config = validate_config();
}

void HeatingController::save_settings()
{
    Preferences prefs;
    prefs.begin("heating", false);

    prefs.putBool ("sys_en", _system_enabled);
    prefs.putFloat("ft_p15", _furnace_temp_p15);
    prefs.putFloat("ft_n15", _furnace_temp_n15);
    prefs.putFloat("ft_off", _furnace_max_off);
    prefs.putBool ("he_cir", _heating_circulation_enabled);
    prefs.putFloat("dhw_min", _dhw_storage_min_temp);
    prefs.putFloat("dhw_max", _dhw_storage_max_temp);
    prefs.putUChar("ts_out", _sensor_outside_id);
    prefs.putUChar("ts_fur", _sensor_furnace_id);
    prefs.putUChar("ts_dhw", _sensor_dhw_storage_id);
    prefs.putBool ("scr_off", _screen_turn_off_enabled);

    prefs.end();
}

bool HeatingController::validate_config()
{
    // Check if all sensors are different
    if (_sensor_furnace_id == _sensor_dhw_storage_id ||
        _sensor_furnace_id == _sensor_outside_id ||
        _sensor_dhw_storage_id == _sensor_outside_id) {
        return false;
    }

    if (_dhw_storage_min_temp >= _dhw_storage_max_temp) return false;

    return true;
}

void HeatingController::change_page(Page page_type)
{
    if (page_type != Page::RedrawPage &&  _current_page != page_type) {
        _current_page = page_type;

        switch (_current_page) {
            #define PAGES_SWITCH(name) \
                case Page::name: \
                    _page_draw = &HeatingController::gui_##name##_draw; \
                    _page_update = &HeatingController::gui_##name##_update; \
                    _on_touch = &HeatingController::gui_##name##_on_touch; \
                    _on_btn_press = &HeatingController::gui_##name##_on_btn_press; \
                    break;
            HC_PAGES_FOR_EACH(PAGES_SWITCH)
            #undef PAGES_SWITCH
            default:
                assert(!"Unknown page!");
                break;
        }
    }

    if (page_type == Page::Settings) {
        _valid_config = false;
        _last_heating_update = 0;
    }

    (this->*_page_draw)();
    _gui_last_update = 0;
    (this->*_page_update)();

    if (page_type == Page::FatalError) for(;;) {
        process_input();
        (this->*_page_update)();
    }
}

bool HeatingController::run_every(unsigned long interval)
{
    if (_gui_last_update == 0 || millis() - _gui_last_update > interval) {
        _gui_last_update = millis();
        if (_gui_last_update == 0) ++_gui_last_update;
        return false;
    }
    return true;
}

void HeatingController::process_screen_timeout()
{
    if (!_screen_turn_off_enabled) return;

    unsigned long screen_timeout = 300 * 1000; // 5 minutes
    if (_screen_on) {
        if (millis() - _last_input > screen_timeout) {
            set_screen_enabled(false);
        }
    } else {
        if (millis() - _last_input <= screen_timeout) {
            set_screen_enabled(true);
        }
    }
}

void HeatingController::set_screen_enabled(bool enable)
{
    if (_screen_on == enable) return;
    _screen_on = enable;
    digitalWrite(TFT_LED_NEN, enable ? LOW : HIGH);
    if (enable) {
        _on_touch = _screen_on_on_touch;
        _on_btn_press = _screen_on_on_btn_press;
    } else {
        _screen_on_on_touch = _on_touch;
        _screen_on_on_btn_press = _on_btn_press;
        _on_touch = &HeatingController::screen_timeout_on_touch;
        _on_btn_press = &HeatingController::screen_timeout_on_btn_press;
    }
}
