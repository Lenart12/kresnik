#ifndef DEFINES_H
#define DEFINES_H

#include <inttypes.h>
#include <Adafruit_ILI9341.h>

// SERIAL_DEBUG {

#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
	#define Sbegin(baud)         Serial.begin(baud)
	#define Sprint(args...)      Serial.print(args) 
	#define Sprintln(args...)    Serial.println(args)
#else
	#define Sprint(args...)
	#define Sprintln(args...)
	#define Sbegin(baud)
#endif
//}

// PINS {
static const uint8_t SD_CS = 4;     // 26

static const uint8_t TFT_LED = 27;  // 12
static const uint8_t TFT_DC = 16;   // 27
static const uint8_t TFT_RST = 17;  // 28
static const uint8_t TFT_CS = 5;    // 29


static const uint8_t SPI_CLK = 18;  // 30
static const uint8_t SPI_MISO = 19; // 31
static const uint8_t SPI_MOSI = 5;  // 37

static const uint8_t I2C_SDA = 21;  // 33
static const uint8_t I2C_SCL = 22;  // 36

static const uint8_t T_DIN = 32;    // 08
static const uint8_t T_CS = 33;     // 09
static const uint8_t T_IRQ = 25;    // 10
static const uint8_t T_DO = 26;     // 11

static const uint8_t BUTTON_0 = 36; // 04
static const uint8_t BUTTON_1 = 39; // 05
static const uint8_t BUTTON_2 = 34; // 06
static const uint8_t BUTTON_3 = 35; // 07

// }

// COLORS {
#define fromRGB(r, g, b) \
(uint16_t)( (((r >> 3) & 0x1f) << 11) | ( ((g >> 2) & 0x3f) << 5) | ((b >> 3) & 0x1f) )

static const uint16_t BUTTON_0_COLOR = fromRGB(  0,   0,   0);
static const uint16_t BUTTON_1_COLOR = fromRGB(255, 255, 255);
static const uint16_t BUTTON_2_COLOR = fromRGB(255, 255,   0);
static const uint16_t BUTTON_3_COLOR = fromRGB(  0,   0, 255);

static const uint16_t BUTTON_COLORS[]{
	BUTTON_0_COLOR,
	BUTTON_1_COLOR,
	BUTTON_2_COLOR,
	BUTTON_3_COLOR
};
// }

// TFT {
static const uint16_t W = 320;
static const uint16_t H = 240;
static const uint8_t TFT_ROTATION = 1;
static const uint32_t TFT_SCREEN_OFF = 30 * 1000; // 30 seconds
//}

// BUTTONS {
static const uint8_t BUTTONS[] = {
	BUTTON_0,
	BUTTON_1,
	BUTTON_2,
	BUTTON_3
};
static const uint16_t DEBOUNCE_TIME = 500;

// }

// ADDRESSES {
static const uint8_t DS2482_0 = 0x18;
static const uint8_t DS2482_1 = 0x19;
static const uint8_t PCF8574_STATUS = 0x20;
static const uint8_t PCF8574_THERMO = 0x21;
static const uint8_t PCF8574_RELAY0 = 0x22;
static const uint8_t PCF8574_RELAY1 = 0x23;
// }

// STATUS LEDS {
#define ON LOW
#define OFF HIGH

enum Status_led : uint8_t{
	power = 0,
	wifi = 1,
	moving_motor = 2,
	night_time = 3,
};
//}

static const int32_t NTP_OFFSET = 3600; // GMT+1
#endif