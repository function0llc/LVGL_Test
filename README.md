# ESP32-S3 ILI9488 Display + Touch Test

This PlatformIO project targets an ESP32-S3 DevKitC wired to an SPI-based 3.5" ILI9488
display and an SPI touch controller (e.g. XPT2046). The firmware brings up LVGL on top
of the Arduino framework, displays a generated demo image, and lets you drag it around
the screen with touch input. A tiny in-tree XPT2046 driver replaces the unavailable
`paulstoffregen/XPT2046_Touchscreen` package to ensure the project builds on Apple
Silicon hosts.

## Project layout

| Path | Purpose |
| ---- | ------- |
| `platformio.ini` | PlatformIO environment definition with Arduino framework, library deps, and default build flags. |
| `include/pin_config.h` | Single source of truth for SPI pin mappings, backlight PWM settings, and raw touch calibration values. |
| `include/lv_conf.h` | Minimal LVGL configuration tuned for the ESP32-S3/ILI9488 target. |
| `include/touch/xpt2046_simple.h` + `src/touch/xpt2046_simple.cpp` | Lightweight, locally-sourced XPT2046 driver used instead of an external PlatformIO package. |
| `src/main.cpp` | LVGL-based UI that initializes the panel, touch controller, and shows a draggable demo image. |

## Usage

1. Update `include/pin_config.h` so the pin numbers and touch calibration constants match your wiring.
2. Connect the display + touch module to the ESP32-S3 DevKitC following the same SPI bus for LCD and touch (separate CS lines). Drive the backlight via `PIN_LCD_BL` or tie it high.
3. Build/flash/monitor with PlatformIO:
   ```
   pio run -t upload
   pio device monitor
   ```
4. On boot you should see a gradient background, a LVGL label, and a generated image tile. Touch anywhere on the panel to reposition the image and confirm coordinate mapping. Raw touch information is easy to add if deeper calibration is required.

> **Note:** `moononournation/GFX Library for Arduino` is pinned to `1.5.6` because newer releases currently require `esp32-hal-periman.h`, which is not shipped with the stable ESP32 Arduino core used by this project.

### Calibrating touch

The default `TOUCH_MIN/MAX_RAW_*` values are placeholders. To fine tune them, temporarily add
`Serial.printf` statements inside `readTouch()` (in `src/main.cpp`) to log `rawX/rawY` while
pressing the extreme edges/corners, then update the constants to match those observed min/max
values. Rebuild and flash to apply the calibration.

## Next steps

- Replace the generated image with a logo or bitmap converted via `lv_img_conv` (drop the output into `src/` and point `lv_img_set_src` to it).
- Feed touch diagnostics (raw + calibrated) to the serial monitor or an on-screen label to simplify calibration.
- Add automated tests or CI builds (e.g., `pio test`, `pio run`) when project scope grows.
