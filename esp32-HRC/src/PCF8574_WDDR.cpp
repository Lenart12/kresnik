#include <PCF8574_WDDR.h>
#include <mutex_util.h>

void PCF8574_WDDR::writeDDR(uint8_t value){
    _DDR = value;
    _PORT = 0xFF;
    i2cLock();
    updateGPIO();
    i2cUnlock();
}