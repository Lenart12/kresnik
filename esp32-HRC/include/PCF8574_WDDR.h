#ifndef PCF8574_WDDR_H
#define PCF8574_WDDR_H

#include <PCF8574.h>

class PCF8574_WDDR : public PCF8574{
public:
	void writeDDR(uint8_t value){
		_DDR = value;
		_PORT = 0xFF;
		updateGPIO();
	}
};

#endif