#include <Arduino.h>
#include <SPI.h>
#include <algorithm>
#include <esp_heap_caps.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "touch/xpt2046_simple.h"

#include "pin_config.h"

#ifndef LCD_SPI_FREQUENCY
#define LCD_SPI_FREQUENCY 40000000
#endif

#ifndef TOUCH_SPI_FREQUENCY
#define TOUCH_SPI_FREQUENCY 2000000
#endif

constexpr size_t LVGL_BUFFER_LINES = 40;
constexpr size_t LVGL_BUFFER_PIXELS = LCD_WIDTH * LVGL_BUFFER_LINES;

constexpr int DEMO_IMG_WIDTH = 200;
constexpr int DEMO_IMG_HEIGHT = 200;

SPIClass touchSPI(FSPI);

Arduino_DataBus *bus = new Arduino_ESP32SPI(
    PIN_LCD_DC,
    PIN_LCD_CS,
    PIN_LCD_SCLK,
    PIN_LCD_MOSI,
    PIN_LCD_MISO,
    FSPI);

Arduino_GFX *gfx = new Arduino_ILI9488_18bit(
    bus,
    PIN_LCD_RST,
    PIN_LCD_BL,
    true /* use BGR */);

SimpleXPT2046 touch(PIN_TOUCH_CS, PIN_TOUCH_IRQ);
bool touchAvailable = false;

struct TouchPoint
{
    bool valid{false};
    int16_t x{0};
    int16_t y{0};
    int16_t rawX{0};
    int16_t rawY{0};
};

static TouchPoint lastTouch;

static lv_color_t *lvBuffer1 = nullptr;
static lv_color_t *lvBuffer2 = nullptr;
static lv_disp_draw_buf_t drawBuffer;
static lv_disp_drv_t dispDriver;
static lv_indev_drv_t indevDriver;
static lv_img_dsc_t demoImage;
static lv_color_t demoImagePixels[DEMO_IMG_WIDTH * DEMO_IMG_HEIGHT];
static lv_obj_t *imageWidget = nullptr;

void initBacklight()
{
    if (PIN_LCD_BL < 0)
    {
        return;
    }

    ledcSetup(BACKLIGHT_PWM_CHANNEL, BACKLIGHT_PWM_FREQUENCY, BACKLIGHT_PWM_RESOLUTION);
    ledcAttachPin(PIN_LCD_BL, BACKLIGHT_PWM_CHANNEL);
    ledcWrite(BACKLIGHT_PWM_CHANNEL, (1 << BACKLIGHT_PWM_RESOLUTION) - 1);
}

void initTouch()
{
    if (PIN_TOUCH_CS < 0)
    {
        Serial.println("Touch CS disabled, skipping touch init");
        return;
    }

    touchSPI.begin(PIN_TOUCH_SCLK, PIN_TOUCH_MISO, PIN_TOUCH_MOSI, PIN_TOUCH_CS);

    if (!touch.begin(touchSPI))
    {
        Serial.println("Touch init failed");
        return;
    }

    touch.setRotation(TOUCH_ROTATION);
    touchAvailable = true;
}

int16_t mapRawToPixel(int16_t raw, int16_t rawMin, int16_t rawMax, int16_t size, bool invert)
{
    if (rawMax == rawMin)
    {
        return 0;
    }

    int32_t clamped = constrain(raw, rawMin, rawMax) - rawMin;
    int32_t span = rawMax - rawMin;
    int32_t px = (clamped * (size - 1)) / span;
    if (invert)
    {
        px = (size - 1) - px;
    }
    return static_cast<int16_t>(px);
}

TouchPoint readTouch()
{
    TouchPoint point;

    if (!touchAvailable)
    {
        return point;
    }

    TS_Point raw = touch.getPoint();
    if (raw.z == 0)
    {
        return point;
    }
    int16_t rx = raw.x;
    int16_t ry = raw.y;

    if (TOUCH_SWAP_XY)
    {
        std::swap(rx, ry);
    }

    point.x = mapRawToPixel(rx, TOUCH_MIN_RAW_X, TOUCH_MAX_RAW_X, LCD_WIDTH, TOUCH_INVERT_X);
    point.y = mapRawToPixel(ry, TOUCH_MIN_RAW_Y, TOUCH_MAX_RAW_Y, LCD_HEIGHT, TOUCH_INVERT_Y);
    point.rawX = raw.x;
    point.rawY = raw.y;
    point.valid = true;
    return point;
}

void lvglFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    const int32_t w = area->x2 - area->x1 + 1;
    const int32_t h = area->y2 - area->y1 + 1;
    gfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(color_p), w, h);
    lv_disp_flush_ready(disp);
}

void lvglTouchRead(lv_indev_drv_t *driver, lv_indev_data_t *data)
{
    TouchPoint point = readTouch();
    if (point.valid)
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = point.x;
        data->point.y = point.y;
        lastTouch = point;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
        lastTouch.valid = false;
    }
}

bool initLVGL()
{
    lv_init();

    lvBuffer1 = static_cast<lv_color_t *>(heap_caps_malloc(LVGL_BUFFER_PIXELS * sizeof(lv_color_t),
                                                          MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    lvBuffer2 = static_cast<lv_color_t *>(heap_caps_malloc(LVGL_BUFFER_PIXELS * sizeof(lv_color_t),
                                                          MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));

    if (!lvBuffer1 || !lvBuffer2)
    {
        Serial.println("LVGL buffer allocation failed");
        return false;
    }

    lv_disp_draw_buf_init(&drawBuffer, lvBuffer1, lvBuffer2, LVGL_BUFFER_PIXELS);

    lv_disp_drv_init(&dispDriver);
    dispDriver.hor_res = LCD_WIDTH;
    dispDriver.ver_res = LCD_HEIGHT;
    dispDriver.flush_cb = lvglFlush;
    dispDriver.draw_buf = &drawBuffer;
    lv_disp_drv_register(&dispDriver);

    lv_indev_drv_init(&indevDriver);
    indevDriver.type = LV_INDEV_TYPE_POINTER;
    indevDriver.read_cb = lvglTouchRead;
    lv_indev_drv_register(&indevDriver);

    return true;
}

void generateDemoImage()
{
    for (int y = 0; y < DEMO_IMG_HEIGHT; ++y)
    {
        for (int x = 0; x < DEMO_IMG_WIDTH; ++x)
        {
            uint8_t r = map(x, 0, DEMO_IMG_WIDTH - 1, 40, 255);
            uint8_t g = map(y, 0, DEMO_IMG_HEIGHT - 1, 40, 200);
            uint8_t b = map((x + y) / 2, 0, (DEMO_IMG_WIDTH + DEMO_IMG_HEIGHT) / 2, 50, 180);

            int dx = x - DEMO_IMG_WIDTH / 2;
            int dy = y - DEMO_IMG_HEIGHT / 2;
            if ((dx * dx + dy * dy) < 40 * 40)
            {
                r = 255;
                g = 128;
                b = 0;
            }

            demoImagePixels[y * DEMO_IMG_WIDTH + x] = lv_color_make(r, g, b);
        }
    }

    demoImage.header.always_zero = 0;
    demoImage.header.w = DEMO_IMG_WIDTH;
    demoImage.header.h = DEMO_IMG_HEIGHT;
    demoImage.header.cf = LV_IMG_CF_TRUE_COLOR;
    demoImage.data_size = sizeof(demoImagePixels);
    demoImage.data = reinterpret_cast<const uint8_t *>(demoImagePixels);
}

void buildUI()
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x202833), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x0C1018), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "ESP32-S3 ILI9488 + LVGL");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 16);

    generateDemoImage();
    imageWidget = lv_img_create(screen);
    lv_img_set_src(imageWidget, &demoImage);
    lv_obj_center(imageWidget);
    lv_obj_set_style_shadow_width(imageWidget, 18, 0);
    lv_obj_set_style_shadow_color(imageWidget, lv_color_hex(0x101010), 0);

    lv_obj_t *hello = lv_label_create(screen);
    lv_label_set_text(hello, "Hello World");
    lv_obj_align(hello, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *hint = lv_label_create(screen);
    lv_label_set_text(hint, "Tap to shift the image");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -16);
}

void updateImageFromTouch()
{
    if (!imageWidget || !lastTouch.valid)
    {
        return;
    }

    int16_t x = constrain(lastTouch.x - DEMO_IMG_WIDTH / 2, 0, LCD_WIDTH - DEMO_IMG_WIDTH);
    int16_t y = constrain(lastTouch.y - DEMO_IMG_HEIGHT / 2, 0, LCD_HEIGHT - DEMO_IMG_HEIGHT);
    lv_obj_set_pos(imageWidget, x, y);
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    if (!gfx->begin(LCD_SPI_FREQUENCY))
    {
        Serial.println("Display init failed");
        while (true)
        {
            delay(500);
        }
    }
    gfx->setRotation(0);

    initBacklight();
    initTouch();

    if (!initLVGL())
    {
        Serial.println("LVGL init failed");
        while (true)
        {
            delay(500);
        }
    }

    buildUI();
}

void loop()
{
    lv_timer_handler();
    updateImageFromTouch();
    delay(5);
}
