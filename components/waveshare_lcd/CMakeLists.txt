idf_component_register(
  SRCS "waveshare_lcd.c"
  INCLUDE_DIRS "."
  REQUIRES esp_lcd driver esp_lcd_touch espressif__esp_lcd_touch_gt911
  PRIV_REQUIRES esp_lcd_st7796
)
