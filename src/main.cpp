#include <Arduino.h>
#include <lvgl_screen_display.h>
#include <touchscreen_driver.h>
#include "touch_event_handler.h"
// #include <touch_calibration.h>
// #include "system_manager.h"
TouchEventHandler touchEventHandler(touchscreenDriver);
void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  display.begin();
  touchscreenDriver.begin();
  touchEventHandler.begin();
  touchEventHandler.attachButtonEvent(objects.button_test);
  // systemManager.init();
}

void loop() {
  // put your main code here, to run repeatedly:
  display.update();
  delay(5);
  // touch_calibration_loop();
}