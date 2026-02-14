 idf_component_register(
  SRCS "lvgl_driver.c"
  INCLUDE_DIRS "."
  REQUIRES driver lvgl esp_lcd esp_timer esp_lcd_touch espressif__esp_lcd_touch_gt911 
)
