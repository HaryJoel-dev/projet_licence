#include "touchscreen_driver.h"
#include "../touch_config.h"
#include <Arduino.h>

Touchscreen_Driver::Touchscreen_Driver() : touchscreen(XPT2046_CS), touchscreenSPI(VSPI) {}
Touchscreen_Driver touchscreenDriver;
void Touchscreen_Driver::initTouchscreen() {
    // Start SPI for the touchscreen
    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touchscreen.begin(touchscreenSPI);
    // Set touchscreen rotation to match display (portrait mode, adjust if needed)
    touchscreen.setRotation(2); // DISPLAY_PORTRAIT_MODE
}

void Touchscreen_Driver::begin() {
    initTouchscreen();
    Serial.println("Touchscreen setup completed.");
}

void Touchscreen_Driver::read(lv_indev_t *indev, lv_indev_data_t *data) {
    static int16_t last_x = 0;
    static int16_t last_y = 0;

    if (touchscreen.touched()) {
        TS_Point p = touchscreen.getPoint();
        int16_t x = p.x;
        int16_t y = p.y;
        int16_t z = p.z;

        // Only process touch if pressure exceeds threshold
        if (z > THRESHOLD_Z) {
            // Map raw coordinates to screen coordinates
            x = map(x, TOUCH_X_MIN, TOUCH_X_MAX, 0, SCREEN_WIDTH);
            y = map(y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, SCREEN_HEIGHT);

            // Ensure coordinates are within screen bounds
            x = constrain(x, 0, SCREEN_WIDTH - 1);
            y = constrain(y, 0, SCREEN_HEIGHT - 1);

            data->point.x = x;
            data->point.y = y;
            data->state = LV_INDEV_STATE_PRESSED;

            last_x = x;
            last_y = y;
        } else {
            data->point.x = last_x;
            data->point.y = last_y;
            data->state = LV_INDEV_STATE_RELEASED;
        }
    } else {
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_RELEASED;
    }
}