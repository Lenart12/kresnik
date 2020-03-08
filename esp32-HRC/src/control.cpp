#include <control.h>
#include <globals.h>
#include <semaphore_util.h>
#include <tasks.h>

float TempatureAddress::read(){
    DallasTemperature *tempature = (lane == 1) ? &tempature_1 : &tempature_2;

    xSemaphoreTake(i2c_semaphore, portMAX_DELAY);
    float f = tempature->getTempC(address);
    xSemaphoreGive(i2c_semaphore);
    return f;
}

void MotorControl::update(){
    if(set_up && enabled){
        float outside_temp = outside_addr.read();
        float circuit_temp = circuit_addr.read();

        if(outside_temp == DEVICE_DISCONNECTED_C || circuit_temp == DEVICE_DISCONNECTED_C){
            LV_LOG_ERROR("Tempature sensor not working");
            status_expander.digitalWrite(Status_led::error, ON);
            return;
        }

        float circuit_correction_temp = target_temp - heating_offset_coeficient * outside_temp;

        float temp_delta = outside_temp - circuit_correction_temp;

        if(temp_delta > 2){
            bool dir = temp_delta > 0;
            uint32_t time
            if(dir){
                temp_delta = (temp_delta < 0) -1* temp_delta : temp_delta;
                time = map(temp_delta, 0, 1, 0, 3000);
                time = (time <= 12000 ) ? time : 12000;
                thermo_expander.digitalWrite((dir) ? id*2 : id*2 + 1, ON);
            }
            else{
                temp_delta = (temp_delta < 0) -1* temp_delta : temp_delta;
                time = map(temp_delta, 0, 1, 0, 3000);
                time = (time <= 12000 ) ? time : 12000;  
            }

            LV_LOG_INFO("Moving motor");
            ticker.once_ms(time, motor_control_task, id);
        }
    }
}

void RelayControl::update(){
    if(set_up && enabled){
        switch(type){
            case RelayControlType::time_of_day:{
                if(time_client.update()){
                    if( ((1 << time_client.getDay()) & days) > 0){
                        uint32_t seconds = time_client.getHours() * 3600 +
                                time_client.getMinutes()      * 60   +
                                time_client.getSeconds();
                        if(seconds > turn_on_seconds && current_state != ON){
                            PCF8574_WDDR expander = (id < 8) ? relay0_expander : relay1_expander;
                            expander.digitalWrite(id % 8, ON);
                            current_state = ON;
                        }
                        else if(seconds > turn_off_seconds && current_state != OFF){
                            PCF8574_WDDR expander = (id < 8) ? relay0_expander : relay1_expander;
                            expander.digitalWrite(id % 8, OFF);
                            current_state = OFF;
                        }
                    }
                }
                break;
            }
            case RelayControlType::tempature:{
                float temp = temp_addr.read();
                if(temp != DEVICE_DISCONNECTED_C){
                    bool new_state = (temp > switch_tempature) ? (!inverted) ? ON : OFF : (!inverted) ? OFF : ON;
                    if(current_state != new_state){
                        current_state = new_state;
                        PCF8574_WDDR expander = (id < 8) ? relay0_expander : relay1_expander;
                        expander.digitalWrite(id % 8, new_state);
                    }
                }
                else{
                    LV_LOG_ERROR("Tempature sensor not working");
                    status_expander.digitalWrite(Status_led::error, ON);
                }
                break;
            }
            case RelayControlType::wifi:{
                break;
            }
        }
    }
}

void Control::update(){
    for(uint8_t i = 0; i < 4; i++){
        motors[i].update();
    }
    for(uint8_t i = 0; i < 16; i++){
        relays[i].update();
    }
}