#ifndef PCF8574_WDDR_H
#define PCF8574_WDDR_H

#include <PCF8574.h>
#include <globals.h>

class PCF8574_WDDR : public virtual PCF8574{
public:
	void writeDDR(uint8_t value){
		_DDR = value;
		_PORT = 0xFF;
		updateGPIO();
	}
protected:
	void updateGPIO() override {
		if(i2c_semaphore == NULL){
			LV_LOG_ERROR("I2C sempahore is NULL");
			return;
		}
		else{
			LV_LOG_TRACE("Taking i2c semaphore");
			xSemaphoreTake(i2c_semaphore, portMAX_DELAY);
			LV_LOG_TRACE("Took i2c semaphore");
			PCF8574::updateGPIO();
			xSemaphoreGive(i2c_semaphore);
			LV_LOG_TRACE("Released i2c semaphore");
		}
		
	}

	void readGPIO() override {
		if(i2c_semaphore == NULL){
			LV_LOG_ERROR("I2C sempahore is NULL");
			return;
		}
		else{
			LV_LOG_TRACE("Taking i2c semaphore");
			xSemaphoreTake(i2c_semaphore, portMAX_DELAY);
			LV_LOG_TRACE("Took i2c semaphore");
			PCF8574::readGPIO();
			xSemaphoreGive(i2c_semaphore);
			LV_LOG_TRACE("Released i2c semaphore");
		}
}
};

#endif