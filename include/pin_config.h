#pragma once

/**
 * Update these pin assignments and calibration constants to reflect your wiring.
 * The defaults target an ESP32-S3-DevKitC-1 where the LCD and touch controller
 * share the same SPI bus.
 */

// LCD SPI interface
constexpr int PIN_LCD_MOSI = 11;
constexpr int PIN_LCD_MISO = 13;      // Optional (only needed for readback)
constexpr int PIN_LCD_SCLK = 12;
constexpr int PIN_LCD_CS   = 10;
constexpr int PIN_LCD_DC   = 7;       //9
constexpr int PIN_LCD_RST  = 6;       //8
constexpr int PIN_LCD_BL   = 5;      // 45 Backlight PWM (set to -1 if tied to VIN)

// Touch controller SPI interface (shares bus by default)
constexpr int PIN_TOUCH_MOSI = 11;    // T_DN PIN_LCD_MOSI;
constexpr int PIN_TOUCH_MISO = 13;    // T_DO PIN_LCD_MISO;
constexpr int PIN_TOUCH_SCLK = 12;    // T_CLK PIN_LCD_SCLK;
constexpr int PIN_TOUCH_CS   = 4;     // T_CS 
constexpr int PIN_TOUCH_IRQ  = -1;   // Optional interrupt pin (-1 to poll)

// Display geometry
constexpr int LCD_WIDTH  = 320;
constexpr int LCD_HEIGHT = 480;
constexpr uint8_t LCD_ROTATION = 2; // 0-3, matches panel orientation (2 = 180°)

// Simple raw touch calibration values (update after running the app)
constexpr int16_t TOUCH_MIN_RAW_X = 200;
constexpr int16_t TOUCH_MAX_RAW_X = 3800;
constexpr int16_t TOUCH_MIN_RAW_Y = 150;
constexpr int16_t TOUCH_MAX_RAW_Y = 3900;
constexpr uint8_t TOUCH_ROTATION  = LCD_ROTATION;   // 0-3, matches display rotation
constexpr bool TOUCH_SWAP_XY      = false;
constexpr bool TOUCH_INVERT_X     = false;
constexpr bool TOUCH_INVERT_Y     = true;

// Backlight PWM settings
constexpr int BACKLIGHT_PWM_CHANNEL = 1;
constexpr int BACKLIGHT_PWM_FREQUENCY = 5000;
constexpr int BACKLIGHT_PWM_RESOLUTION = 8;
