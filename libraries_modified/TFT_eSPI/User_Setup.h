// User_Setup.h - GroundStation ELRS Backpack
// TFT_eSPI custom configuration validated for Marcelo Grippo GroundStation V2.5.1
// Display: ST7789 1.9 inch, 172 x 320, used in landscape rotation by firmware.

#define USER_SETUP_INFO "GroundStation_ELRS_Backpack_ST7789_172x320"

#define ST7789_DRIVER
#define CGRAM_OFFSET

#define TFT_WIDTH  172
#define TFT_HEIGHT 320

// SPI display pins for ESP32 NodeMCU 38-pin board
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4

// Backlight is controlled from the sketch using PWM on GPIO25.
// Do not enable TFT_BL here unless you intentionally move brightness control into TFT_eSPI.
// #define TFT_BL 25
// #define TFT_BACKLIGHT_ON HIGH

// Fonts used by the project
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI speed validated with the selected ST7789 display
#define SPI_FREQUENCY       27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY 2500000
