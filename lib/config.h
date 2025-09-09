#pragma once
//SD card GPIOs
// #define MISO_GPIO    19
// #define MOSI_GPIO    23
// #define SCK_GPIO     18
// #define CS_GPIO       5
#define MISO_GPIO  13
#define MOSI_GPIO  11
#define SCK_GPIO   12
#define CS_GPIO    10
//LED RGB
#define LED_PIN    48     // Pin à laquelle est connectée ta LED RGB
#define NUM_LEDS   1     // Nombre de LEDs
#define COLOR_ORDER GRB  // Ordre des couleurs selon ta LED
#define STABILITY_DELAY 3000
//Screen config 
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))