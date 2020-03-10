#ifndef SEMAPHORE_UTIL_H
#define SEMAPHORE_UTIL_H

#include <globals.h>
#include <Arduino.h>

template<class T>
void setWithSemaphore(T &variable, T value, SemaphoreHandle_t handle){
	LV_LOG_TRACE("Taking semaphore");
	xSemaphoreTake(handle, portMAX_DELAY);
	LV_LOG_TRACE("Took semaphore");
	variable = value;
	xSemaphoreGive(handle);
	LV_LOG_TRACE("Gave semaphore");
}

template<class T>
T getWithSemaphore(T &variable, SemaphoreHandle_t handle){
    T out;
	LV_LOG_TRACE("Taking semaphore");
	xSemaphoreTake(handle, portMAX_DELAY);
	LV_LOG_TRACE("Took semaphore");
	out = variable;
	xSemaphoreGive(handle);
	LV_LOG_TRACE("Gave semaphore");
    return out;
}

void giveSemaphore(SemaphoreHandle_t *semaphore);
#endif