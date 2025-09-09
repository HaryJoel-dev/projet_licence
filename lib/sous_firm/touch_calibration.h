#ifndef TOUCH_CALIBRATION_H
#define TOUCH_CALIBRATION_H

#include <Arduino.h>

// Initialise la calibration (à appeler dans setup)
void touch_calibration_setup();

// Boucle de calibration (à appeler dans loop)
void touch_calibration_loop();

#endif // TOUCH_CALIBRATION_H
