idf_component_register(
  SRCS "deck_cdc.c"
  INCLUDE_DIRS "."
  REQUIRES esp_lcd esp_timer esp_tinyusb usb deck_gl
)   
