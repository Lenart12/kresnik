#ifndef MUTEX_UTIL_H
#define MUTEX_UTIL_H

#include <globals.h>
#include <Arduino.h>

template<class T>
void setWithMutex(T &variable, T value, SemaphoreHandle_t handle){
	LV_LOG_TRACE("Taking mutex");
	xSemaphoreTake(handle, portMAX_DELAY);
	LV_LOG_TRACE("Took mutex");
	variable = value;
	xSemaphoreGive(handle);
	LV_LOG_TRACE("Gave mutex");
}

template<class T>
T getWithMutex(T &variable, SemaphoreHandle_t handle){
    T out;
	LV_LOG_TRACE("Taking mutex");
	xSemaphoreTake(handle, portMAX_DELAY);
	LV_LOG_TRACE("Took mutex");
	out = variable;
	xSemaphoreGive(handle);
	LV_LOG_TRACE("Gave mutex");
    return out;
}

void unlockMutex(SemaphoreHandle_t *handle);

void i2cLock();

void i2cUnlock();

#endif