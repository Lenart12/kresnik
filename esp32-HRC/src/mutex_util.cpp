#include <mutex_util.h>

void unlockMutex(SemaphoreHandle_t *handle){
	xSemaphoreGive(*handle);
}

void i2cLock(){
	LV_LOG_TRACE("Locking I2C");
	xSemaphoreTake(i2c_mutex, portMAX_DELAY);
}

void i2cUnlock(){
	LV_LOG_TRACE("Unlocking I2C");
	xSemaphoreGive(i2c_mutex);
}