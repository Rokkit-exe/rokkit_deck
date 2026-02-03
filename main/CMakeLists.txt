idf_component_register(
  SRCS "rokkit-deck.c"
  INCLUDE_DIRS "."
  REQUIRES driver lvgl deck_gl st7796_lcd esp_lcd
)
