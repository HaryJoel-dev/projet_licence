#ifndef LVGL_DISPLAY_H_
#define LVGL_DISPLAY_H_

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "../lvgl_user_interface/src/ui/ui.h"  // EEZ export
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// Configuration de l'écran
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

class LVGL_Display {
public:
    LVGL_Display();                 // Constructeur
    void begin();                   // Initialise l'écran et LVGL
    void update();                  // Met à jour LVGL
private:
    TFT_eSPI tft;
    lv_display_t *disp;
    uint8_t *draw_buf;
    uint32_t lastTick;
};

extern LVGL_Display display;

#endif
