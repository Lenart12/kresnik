#include <semaphore_util.h>

void giveSemaphore(SemaphoreHandle_t *semaphore){
	xSemaphoreGive(*semaphore);
}