#ifndef TOUCHSCREEN_DRIVER_H_
#define TOUCHSCREEN_DRIVER_H_

#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <lvgl.h>

// Touchscreen pin configuration
#define XPT2046_MOSI 2   // T_DIN
#define XPT2046_MISO 41  // T_OUT
#define XPT2046_CLK  42  // T_CLK
#define XPT2046_CS   1   // T_CS

// Predefined calibration values (to be adjusted based on your touchscreen)
#define THRESHOLD_Z 500

#define VSPI FSPI // Use VSPI for ILI9341, HSPI for ILI9488
class Touchscreen_Driver {
public:
    Touchscreen_Driver();           // Constructor
    void begin();                   // Initialize touchscreen
    void read(lv_indev_t *indev, lv_indev_data_t *data); // Read touchscreen data for LVGL

private:
    XPT2046_Touchscreen touchscreen;
    SPIClass touchscreenSPI;
    void initTouchscreen();         // Initialize SPI and touchscreen
};
extern Touchscreen_Driver touchscreenDriver;
#endif