#include <PCF8574_WDDR.h>
#include <mutex_util.h>

void PCF8574_WDDR::writeDDR(uint8_t value){
    _DDR = value;
    _PORT = 0xFF;
    updateGPIO();
}

void PCF8574_WDDR::updateGPIO() {
    i2cLock();
    PCF8574::updateGPIO();
    i2cUnlock();
}

void PCF8574_WDDR::readGPIO() {
    i2cLock();
    PCF8574::readGPIO();
    i2cUnlock();
}