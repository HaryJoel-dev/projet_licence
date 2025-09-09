#ifndef TOUCH_EVENT_HANDLER_H_
#define TOUCH_EVENT_HANDLER_H_

#include <lvgl.h>
#include "touchscreen_driver.h"

class TouchEventHandler {
public:
    TouchEventHandler(Touchscreen_Driver &touch); // Constructor takes Touchscreen_Driver reference
    void begin();                                // Initialize touchscreen input for LVGL
    void attachButtonEvent(lv_obj_t *button, lv_event_cb_t event_cb = nullptr); // Attach event callback to a button
private:
    Touchscreen_Driver &touch;                   // Reference to Touchscreen_Driver
    lv_indev_t *indev;                           // LVGL input device
    static void defaultButtonEventCb(lv_event_t *e); // Default event callback for buttons
};

#endif