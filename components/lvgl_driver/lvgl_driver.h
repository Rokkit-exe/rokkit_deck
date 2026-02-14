#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "lvgl.h"

lv_display_t *lvgl_create_display(esp_lcd_panel_handle_t panel, uint16_t width,
                                  uint16_t height);
lv_indev_t *lvgl_create_touch(esp_lcd_touch_handle_t touch_handle,
                              uint16_t lcd_width, uint16_t lcd_height);
