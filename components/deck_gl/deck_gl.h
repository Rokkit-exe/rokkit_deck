#pragma once

#include "esp_lcd_panel_io.h"
#include "lvgl.h"

typedef struct {
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel_handle;
    int hor_res;
    int ver_res;
} deck_config_t;


typedef struct DeckGL {
    size_t buffer_size;
    void* draw_buffer;
    lv_display_t* display;
    int hor_res;
    int ver_res;
} DeckGL;

DeckGL* init_deck_display(deck_config_t *cfg);
void create_button_grid(DeckGL* deck);
