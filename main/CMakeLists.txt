idf_component_register(
  SRCS "rokkit-deck.c"
  INCLUDE_DIRS "."
  REQUIRES driver lvgl lvgl_driver bsp_waveshare esp_lcd deck_gl deck_hid
)
