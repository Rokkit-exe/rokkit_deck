idf_component_register(
  SRCS "st7796_lcd.c"
  INCLUDE_DIRS "."
  REQUIRES esp_lcd driver
  PRIV_REQUIRES esp_lcd_st7796
)
