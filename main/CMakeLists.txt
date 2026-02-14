idf_component_register(
  SRCS "rokkit-deck.c"
  INCLUDE_DIRS "."
  REQUIRES driver lvgl deck_gl bsp_waveshare esp_lcd
)
