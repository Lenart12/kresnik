/* 
 *  This is the default license template.
 *  
 *  File: defines.h
 *  Author: Lenart
 *  Copyright (c) 2021-2024 Lenart
 *  
 *  To edit this license information: Press Ctrl+Shift+P and press 'Create new License Template...'.
 */

#ifndef DEFINES_H
#define DEFINES_H

#include <inttypes.h>

#ifndef BK_FW_VERSION
#define BK_FW_VERSION debug
#endif
#define CSTR_FB1(x) #x
#define CSTR_FB(x) CSTR_FB1(x)
#define BK_FW_VERSION_STR CSTR_FB(BK_FW_VERSION)


// PINS {
static const uint8_t SD_CS = 4;     // 26

static const uint8_t TFT_LED_NEN = 27;  // 12

static const uint8_t SPI_CLK = 18;  // 30
static const uint8_t SPI_MISO = 19; // 31
static const uint8_t SPI_MOSI = 23;  // 37

static const uint8_t I2C_SDA = 21;  // 33
static const uint8_t I2C_SCL = 22;  // 36

static const uint8_t T_DIN = 32;    // 08
static const uint8_t T_CS = 33;     // 09
static const uint8_t T_IRQ = 25;    // 10
static const uint8_t T_DO = 26;     // 11

static const uint8_t BTN_B_PIN = 36; // 04
static const uint8_t BTN_W_PIN = 39; // 05
static const uint8_t BTN_Y_PIN = 34; // 06
static const uint8_t BTN_C_PIN = 35; // 07

static uint16_t TOUCH_CALIBRATION[5] = { 160, 3543, 289, 3503, 1 };
// }



// BUTTONS {
static const uint8_t BTN_PINS[] = {
	BTN_B_PIN,
	BTN_W_PIN,
	BTN_Y_PIN,
	BTN_C_PIN
};
// }

// ADDRESSES {
static const uint8_t DS2482_0 = 0x19;
static const uint8_t DS2482_1 = 0x18;
static const uint8_t PCF8574_STATUS = 0x20;
static const uint8_t PCF8574_THERMO = 0x21;
static const uint8_t PCF8574_RELAY0 = 0x22;
static const uint8_t PCF8574_RELAY1 = 0x23;
// }

#endif