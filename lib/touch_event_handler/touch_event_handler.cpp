#include "touch_event_handler.h"
#include <Arduino.h>

TouchEventHandler::TouchEventHandler(Touchscreen_Driver &touch) : touch(touch), indev(nullptr) {}
void TouchEventHandler::begin() {
    // Register touchscreen as LVGL input device
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, [](lv_indev_t *indev, lv_indev_data_t *data) {
        Touchscreen_Driver *driver = (Touchscreen_Driver *)lv_indev_get_user_data(indev);
        driver->read(indev, data);
    });
    lv_indev_set_user_data(indev, &touch);

    Serial.println("Touch event handler setup completed.");
}

void TouchEventHandler::defaultButtonEventCb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_PRESSED) {
        // Change style when pressed
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1E90FF), LV_STATE_FOCUSED | LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(btn, LV_OPA_80, LV_STATE_FOCUSED | LV_STATE_PRESSED);
        Serial.println("Button pressed!");
    } else if (code == LV_EVENT_RELEASED) {
        // Reset style when released
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1E90FF), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_STATE_DEFAULT);
        Serial.println("Button released!");
    } else if (code == LV_EVENT_CLICKED) {
        // Handle button click
        Serial.println("Button clicked!");
    }
}

void TouchEventHandler::attachButtonEvent(lv_obj_t *button, lv_event_cb_t event_cb) {
    if (button != nullptr) {
        // Attach provided callback or default callback
        lv_obj_add_event_cb(button, event_cb ? event_cb : defaultButtonEventCb, LV_EVENT_ALL, nullptr);
    } else {
        Serial.println("Error: Button object is null!");
    }
}