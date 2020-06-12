#include <control.h>
#include <globals.h>
#include <mutex_util.h>
#include <tasks.h>
#include <EEPROM.h>

// Reads the tempature of the sensor
float TempatureAddress::read(){
    // Use the right expander
    DallasTemperature *tempature = (lane == 1) ? &tempature_1 : &tempature_2;

    i2cLock();
    // Read tempature
    float f = tempature->getTempC(address);
    i2cUnlock();

    return f;
}

// Reads the value of the relay expander
bool relayRead(uint8_t pin){;
    // Use the right expander
	PCF8574_WDDR *expander = (pin < 8) ? &relay0_expander : &relay1_expander;

    // Return the pin value
	return expander->digitalRead(pin % 8) == ON;
}

// Writes the value to the relay
void relayWrite(uint8_t pin, bool value){
    String log = ((value == true) ? "Opening" : "Closing") + (String)" relay " + pin;
    LV_LOG_INFO(log.c_str());

    // Set the status led for burner or pump
    if(pin == Relay_pin::burner) status_expander.digitalWrite(Status_led::burner_s, (value == true) ? ON : OFF);
    else if(pin == Relay_pin::hot_water_pump) status_expander.digitalWrite(Status_led::hot_water_pump_s, (value == true) ? ON : OFF);
    else if(pin == Relay_pin::circulator_pump) status_expander.digitalWrite(Status_led::circulator_s, (value == true) ? ON : OFF);

    // Use the right expander
	PCF8574_WDDR *expander = (pin < 8) ? &relay0_expander : &relay1_expander;

    // Set the pin
	expander->digitalWrite(pin % 8, (value == true) ? ON : OFF);
}

// Move the specified motor in the direction for a certain time
void move_motor(uint8_t motor, MotorDirection direction, uint32_t time_ms){
    // Try to move the motor for 10ms
    if( xSemaphoreTake(motor_mutexes[motor], 10 / portTICK_PERIOD_MS ) == pdTRUE ){
        String log = ((direction == MotorDirection::dir_open) ? "Opening" : "Closing") + (String)" motor " + motor + " for " + time_ms + "ms";
        LV_LOG_INFO(log.c_str());

        // Move the motor
        uint8_t dir = (direction == MotorDirection::dir_open) ? 0 : 1;
        thermo_expander.digitalWrite( motor * 2 + dir, ON);
        thermo_expander.digitalWrite( motor * 2 + (1 - dir), OFF);
        // Set the moving motor status led
        status_expander.digitalWrite(Status_led::moving_motor, ON);

        // Start the timer to turn off the motor
        motor_tickers[motor].once_ms(time_ms, stop_motor, motor);
    }
}

// Function that stops the specified motor
void stop_motor(uint8_t motor){
    // Turn off both motor directions 
    thermo_expander.digitalWrite(motor * 2, OFF);
    thermo_expander.digitalWrite(motor * 2 + 1, OFF);

    // Turn off the moving motor status led if no motor is moving
    status_expander.digitalWrite(Status_led::moving_motor, ( thermo_expander.read() != 0xFF) ? ON : OFF);

    String log = "Stopping motor " + (String)motor;
    LV_LOG_INFO(log.c_str());

    // Unlock the motors to move after two minutes
    motor_tickers[motor].once_ms(120000, unlockMutex, &motor_mutexes[motor]);
}

// Setup function for all the control related things
void Control::setup(){
    thermo_expander.write(0xFF);
    relay0_expander.write(0xFF);
    relay1_expander.write(0xFF);

    LV_LOG_INFO("Closing motors and reseting underfloor pumps");

    // Close all the motors and turn on/off all the pumps
    for(uint8_t i = 0; i < 4; i++){
        move_motor(i, MotorDirection::dir_close, 120000);
        relayWrite(Relay_pin::underfloor_pump_0 + i, underfloor_pump_enabled[i]);
    }

    // Turn off the circulator pump
    relayWrite(Relay_pin::circulator_pump, false);
}

// Update task
void Control::update(){
    // Read all the tempatures
    xSemaphoreTake(tempature_mutex, portMAX_DELAY);
    float boiler_tempature = boiler_temp[0];
    float hot_water_container_tempature = hot_water_container_temp[0];
    float enviroment_tempature = enviroment_temp[0];
    float underfloor_tempature[4];
    for(uint8_t i = 0; i < 4; i++){
        underfloor_tempature[i] = underfloor_temp[i][0];
    }
    float solar_collector_tempature = solar_collector_temp[0];
    float solar_tank_tempature = solar_tank_temp[0];
    float heat_exchanger_tempature = heat_exchanger_temp[0];
    xSemaphoreGive(tempature_mutex);

    static int i = 0;

    // Calculate tempature correction
    float underfloor_temp_correction = -(enviroment_tempature * tempature_slope);

    // Adjust for night time
    static bool night_time = false;
    if(night_time){
        underfloor_temp_correction -= night_time_correction;
    }

    // Log tempatures
    String tempatures = (String)i++ + ':' + (String)boiler_tempature + ' ' + hot_water_container_tempature + ' ' +
    enviroment_tempature + ' ' + underfloor_tempature[0] + ' '
	+ underfloor_tempature[1] + ' ' + underfloor_tempature[2] + ' '
    + underfloor_tempature[3] + ' ' + solar_collector_tempature + ' ' +
	solar_tank_tempature + ' ' + heat_exchanger_tempature + ' ' + underfloor_temp_correction;
    LV_LOG_TRACE(tempatures.c_str());

    // Turn on the boiler if it is required
    static bool need_for_heat = false;
    if(need_for_heat && burner_enabled && boiler_tempature < boiler_min_temp){
        config.burner_seconds++;
        if(relayRead(Relay_pin::burner) != true){
            LV_LOG_INFO("Boiler tempature too low - turning on");
            relayWrite(Relay_pin::burner, true);
        }
    }
    else if(boiler_tempature > boiler_max_temp){
        if(relayRead(Relay_pin::burner) != false){
            LV_LOG_INFO("Boiler tempature too high - turning off");
            relayWrite(Relay_pin::burner, false);
        }
        need_for_heat = false;
    }

    // Turn hot water pump on/off if required
    bool hwp_state = relayRead(Relay_pin::hot_water_pump);

    if(hot_water_pump_enabled && !hwp_state &&
        hot_water_container_tempature < hot_water_min_temp &&
        burner_enabled &&
        !night_time &&
        boiler_tempature > hot_water_container_tempature + 5.00f){
        LV_LOG_INFO("Hot water container tempature too low - turning on hwp and reseting underfloor pumps");
        for(int i = 0; i < 4; i++){
            relayWrite(Relay_pin::underfloor_pump_0 + i, false);
        }
        relayWrite(Relay_pin::hot_water_pump, true);
        hwp_state = true;
        need_for_heat = true;
    }
    else if(hwp_state && (hot_water_container_tempature > hot_water_max_temp || boiler_tempature - hot_water_container_tempature < 3.00f)){
        LV_LOG_INFO("Hot water container tempature too high - turning off and reseting underfloor pumps");
        relayWrite(Relay_pin::hot_water_pump, false);
        hwp_state = false;
        need_for_heat = false;
        for(int i = 0; i < 4; i++){
            relayWrite(Relay_pin::underfloor_pump_0 + i, underfloor_pump_enabled[i]);
        }
    }

    // Open/close the motors if needed
    for(uint8_t i = 0; i < 4; i++){
        if(underfloor_tempature[i] >= 40.00f){
            String log = (String)"Underfloor circuit " + i + " tempature over 40C! (" + underfloor_tempature[i] + "C)";
            LV_LOG_ERROR(log.c_str())
        }
        if(!relayRead(Relay_pin::underfloor_pump_0 + i) || !underfloor_pump_enabled[i])
            continue;

        need_for_heat = true;
        
        if(!hwp_state){
            if(xSemaphoreTake(motor_mutexes[i], 10 / portTICK_PERIOD_MS)){
                xSemaphoreGive(motor_mutexes[i]);

                // Calculate wanted tempature and adjust the motor

                float wanted_tempature = underfloor_wanted_temp[i] + underfloor_temp_correction;
                if(underfloor_tempature[i] >= wanted_tempature){
                    uint32_t delta_time = (underfloor_tempature[i] - wanted_tempature) * 3 * 1000;
                    delta_time = (delta_time > 12000) ? 12000 : delta_time;
                    if(delta_time > 1000){
                        String log = (String)"Motor " + i + " needs to correct for " + (underfloor_tempature[i] - wanted_tempature) + " degrees";
                        LV_LOG_INFO(log.c_str());
                        move_motor(i, MotorDirection::dir_close, delta_time);
                    }
                }
                else{
                    uint32_t delta_time = (wanted_tempature - underfloor_tempature[i]) * 2 * 1000;
                    delta_time = (delta_time > 10000) ? 10000 : delta_time;
                    if(delta_time > 1000){
                        String log = (String)"Motor " + i + " needs to correct for " + (wanted_tempature - underfloor_tempature[i]) + " degrees";
                        LV_LOG_INFO(log.c_str());
                        move_motor(i, MotorDirection::dir_open, delta_time);
                    }
                }
            }
        }
    }

    // Turn on/off inter tank pump if needed
    static bool force_inter_tank_pump = false;
    if(solar_pump_enabled){
        if(inter_tank_pump_enabled){
            bool itp_state = relayRead(Relay_pin::inter_tank_pump);
            if((!itp_state && solar_tank_tempature > inter_tank_trigger_tempature &&
                solar_tank_tempature - hot_water_container_tempature > inter_tank_temp_difference + 2.00f) ||
                (!itp_state && solar_tank_tempature > max_solar_temp - 5.00f)){
                LV_LOG_INFO("Turning on inter tank pump");
                relayWrite(Relay_pin::inter_tank_pump, true);
                itp_state = true;
            }
            if(itp_state && !force_inter_tank_pump &&
                solar_tank_tempature - hot_water_container_tempature < inter_tank_temp_difference){
                LV_LOG_INFO("Turning off inter tank pump");
                relayWrite(Relay_pin::inter_tank_pump, false);
            }
        }

        bool sp_state = relayRead(Relay_pin::solar_pump);
        if(!sp_state && heat_exchanger_tempature < -18.00f){
            LV_LOG_INFO("Heat exchanger tempature too low ( < -18C ) - turning on solar pump");
            relayWrite(Relay_pin::solar_pump, true);
            sp_state = true;
        }
        if(solar_pump_enabled){
            if(!sp_state && heat_exchanger_tempature < max_solar_temp &&
                heat_exchanger_tempature + collector_temp_difference + 2.00f < solar_collector_tempature){
                LV_LOG_INFO("Turning on solar pump");
                relayWrite(Relay_pin::solar_pump, true);
            }
            else if(sp_state && heat_exchanger_tempature + collector_temp_difference > solar_collector_tempature){
                LV_LOG_INFO("Turning off solar pump");
                relayWrite(Relay_pin::solar_pump, false);
            }
        }
    }

    // Update every minute
    if(getWithMutex(minute_update, timing_mutex)){
        setWithMutex(minute_update, false, timing_mutex);

        // If ntp is working
        tm time;
        if(getLocalTime(&time, 10)){
            // Calcualte total minutes in the day
            uint16_t minutes = time.tm_hour * 60 + time.tm_min;

            // Check for night time
            bool new_night_time = false;
            if((minutes > night_time_start && minutes <= 24 * 60) ||
            (minutes < night_time_end   && minutes > 0))
                new_night_time = true;
            if(new_night_time != night_time){
                night_time = new_night_time;
                status_expander.digitalWrite(Status_led::night_time_s, night_time);
            }

            // Turn on the circulator if its time
            if(circulator_pump_enabled){
                bool c_state = relayRead(Relay_pin::circulator_pump);

                bool c_enabled = false;
                for(uint8_t i = 0; i < 8; i++){
                    if(circulator_start_time[i] < 60 * 24 + 00 &&
                     minutes >= circulator_start_time[i] &&
                     minutes < circulator_start_time[i] + circulator_duration){
                         c_enabled = true;
                         break;
                    }
                }
                if(c_state != c_enabled){
                    String log = ((c_enabled) ? "Opening" : "Closing") + (String)" circulator";
                    LV_LOG_INFO(log.c_str());
                    relayWrite(Relay_pin::circulator_pump, c_enabled);
                }
            }

            // Turn on/off inter tank pump
            bool itp_state = relayRead(Relay_pin::inter_tank_pump);
            if(minutes == 60 * 3 + 1 ||
               minutes == 60 * 3 + 2){
                if(!itp_state){
                    LV_LOG_INFO("Forcing inter tank pump on");
                    relayWrite(Relay_pin::inter_tank_pump, true);
                }
                force_inter_tank_pump = true;
            }
            else{
                if(itp_state){
                    LV_LOG_INFO("Forcing inter tank pump off");
                    relayWrite(Relay_pin::inter_tank_pump, false);
                }
                force_inter_tank_pump = false;
            }
        }

        if(relayRead(Relay_pin::solar_pump)){
            config.solar_minutes++;
        }
        EEPROM.put(0, config);
        EEPROM.commit();
    }
}

// Function that updates pumps
void Control::update_pumps(){
    if(!relayRead(Relay_pin::hot_water_pump)){
        for(uint8_t i = 0; i < 4; i++){
            bool uf_state = relayRead(Relay_pin::underfloor_pump_0 + i);
            if(!uf_state && underfloor_pump_enabled[i]){
                stop_motor(i);
                motor_tickers[i].detach();
                xSemaphoreGive(motor_mutexes[i]);
                move_motor(i, MotorDirection::dir_close, 120 * 1000);
            }
            if(uf_state != underfloor_pump_enabled[i]){
                relayWrite(Relay_pin::underfloor_pump_0 + i, underfloor_pump_enabled[i]);
            }
        }
    }
    if(!solar_pump_enabled)
        relayWrite(Relay_pin::solar_pump, false);
    if(!circulator_pump_enabled)
        relayWrite(Relay_pin::circulator_pump, false);
    if(!inter_tank_pump_enabled)
        inter_tank_pump_enabled = false;
}

void Control::load_default(){
    burner_enabled = false;
    boiler_min_temp = 45.00f;
    boiler_max_temp = 75.00f;

    hot_water_min_temp = 50.00f;
    hot_water_max_temp = 60.00f;

    underfloor_wanted_temp[0] = 22.00f;
    underfloor_wanted_temp[1] = 22.00f;
    underfloor_wanted_temp[2] = 22.00f;
    underfloor_wanted_temp[3] = 22.00f;

    tempature_slope = 0.25f;

    underfloor_pump_enabled[0] = true;
    underfloor_pump_enabled[1] = true;
    underfloor_pump_enabled[2] = true;
    underfloor_pump_enabled[3] = true;

    circulator_start_time[0] = 60 *      6 + 00;
    circulator_start_time[1] = 60 *      6 + 30;
    circulator_start_time[2] = 60 *      7 + 00;
    circulator_start_time[3] = 60 *     11 + 00;
    circulator_start_time[4] = 60 *     14 + 30;
    circulator_start_time[5] = 60 *     19 + 00;
    circulator_start_time[6] = 60 *     19 + 30;
    circulator_start_time[7] = 60 *     24 + 00;

    circulator_pump_enabled = true;

    circulator_duration = 3;

    night_time_correction = 2.00f;
    night_time_start = 60 *     21 + 30;
    night_time_end =   60 *      5 + 00;

    max_solar_temp = 98.00f;
    collector_temp_difference = 7.00f;
    inter_tank_temp_difference = 6.00f;

    solar_pump_enabled = true;
    inter_tank_pump_enabled = true;

    vacation = false;

    memcpy(boiler_addr.address, "\x10\xD9\x61\x3B\x00\x08\x00\xDF", 8);
    boiler_addr.lane = 1;

    memcpy(hot_water_container_addr.address, "\x10\x52\xA5\xC4\x00\x08\x00\x1B", 8);
    hot_water_container_addr.lane = 1;

    memcpy(enviroment_addr.address, "\x10\xBE\x7E\x8D\x00\x08\x00\x12", 8);
    enviroment_addr.lane = 1;

    memcpy(underfloor_addr[0].address, "\x10\xF9\x32\x80\x00\x08\x00\xB7", 8);
    underfloor_addr[0].lane = 2;

    memcpy(underfloor_addr[1].address, "\x10\x31\x37\x80\x00\x08\x00\x5B", 8);
    underfloor_addr[1].lane = 2;

    memcpy(underfloor_addr[2].address, "\x10\xD8\x73\x80\x00\x08\x00\x12", 8);
    underfloor_addr[2].lane = 2;

    memcpy(underfloor_addr[3].address, "\x10\x05\x4C\x80\x00\x08\x00\x77", 8);
    underfloor_addr[3].lane = 2;

    memcpy(solar_collector_addr.address, "\x28\x74\x55\x51\x00\x00\x00\x4A", 8);
    solar_collector_addr.lane = 1;

    memcpy(solar_tank_addr.address, "\x10\xA3\xA3\xBA\x00\x08\x00\x7F", 8);
    solar_tank_addr.lane = 1;

    memcpy(heat_exchanger_addr.address, "\x10\x8B\x4A\x09\x00\x00\x00\x2E", 8);
    heat_exchanger_addr.lane = 1;
}