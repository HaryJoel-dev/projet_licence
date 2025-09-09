#include <Arduino.h>
#include <lvgl_screen_display.h>
#include <touch_calibration.h>
#include "system_manager.h"
LVGL_Display display;
void setup() {
  // put your setup code here, to run once:
  //display.begin();
  // touch_calibration_setup();
  Serial.begin(115200);
  systemManager.init();
}

void loop() {
  // put your main code here, to run repeatedly:
  //display.update();
  // touch_calibration_loop();
}