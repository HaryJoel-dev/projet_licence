#include "lvgl_screen_display.h"
#include <Arduino.h>

LVGL_Display::LVGL_Display() : tft(), disp(nullptr), draw_buf(nullptr), lastTick(0) {}

void LVGL_Display::begin() {
    Serial.begin(115200);
    delay(500);
    // Initialisation de LVGL
    lv_init();
    // Initialisation du buffer
    draw_buf = new uint8_t[DRAW_BUF_SIZE];
    // Création du display LVGL avec TFT_eSPI
    disp = lv_tft_espi_create(SCREEN_HEIGHT, SCREEN_WIDTH, draw_buf, DRAW_BUF_SIZE);
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

    Serial.println();
    Serial.println("LVGL Setup Completed.");
    delay(500);

    // Intégration de l'interface EEZ Studio
    ui_init();
}

void LVGL_Display::update() {
    lv_tick_inc(millis() - lastTick);
    lastTick = millis();
    lv_timer_handler();
}
