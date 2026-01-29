#pragma once

/* Minimal LVGL configuration tuned for the ESP32-S3 ILI9488 test app. */

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1

#define LV_HOR_RES_MAX 320
#define LV_VER_RES_MAX 480

#define LV_MEM_SIZE (48U * 1024U)

#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_INFO

#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#define LV_USE_USER_DATA 1

#define LV_FONT_DEFAULT &lv_font_montserrat_16

#define LV_FONT_MONTSERRAT_16 1

#define LV_USE_PERF_MONITOR 1

#define LV_USE_LARGE_COORD 0
