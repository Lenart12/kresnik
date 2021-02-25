#define ILI9341_DRIVER

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5  // Chip select control pin
#define TFT_DC   16  // Data Command control pin
#define TFT_RST  17  // Reset pin (could connect to RST pin)

#define TOUCH_CS 33     // Chip select pin (T_CS) of touch screen

#define SPI_FREQUENCY  27000000 // Actually sets it to 26.67MHz = 80/3
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000