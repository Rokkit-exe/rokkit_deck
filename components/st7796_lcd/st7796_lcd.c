#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_st7796.h"
#include "stdlib.h"
#include "st7796_lcd.h"

st7796_lcd_handle_t* init_st7796_lcd(st7796_lcd_config_t *config) {
  // 1. Initialize SPI Bus
  spi_bus_config_t buscfg = {
      .sclk_io_num = config->spi_sclk,
      .mosi_io_num = config->spi_mosi,
      .miso_io_num = -1, // Not used for display
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = config->lcd_hor_res * config->lcd_ver_res * sizeof(uint16_t),
  };
  ESP_ERROR_CHECK(spi_bus_initialize(config->lcd_host, &buscfg, SPI_DMA_CH_AUTO));

  // 2. Install Panel IO
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_spi_config_t io_config = {
      .dc_gpio_num = config->lcd_dc,
      .cs_gpio_num = config->lcd_cs,
      .pclk_hz = 40 * 1000 * 1000, // 40MHz
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
      .spi_mode = 0,
      .trans_queue_depth = 10,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)config->lcd_host, &io_config, &io_handle));

  // 3. Install ST7796 Panel Driver
  esp_lcd_panel_handle_t panel_handle = NULL;
  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = config->lcd_rst,
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
      .bits_per_pixel = 16,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle));
  // 4. Reset and Init
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

  // Force 16-bit color if it's struggling
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, false));
  
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

  vTaskDelay(pdMS_TO_TICKS(100));
    // 5. Turn on Backlight
  gpio_set_direction(config->lcd_bl, GPIO_MODE_OUTPUT);
  gpio_set_level(config->lcd_bl, 1);

  st7796_lcd_handle_t *lcd = calloc(1, sizeof(st7796_lcd_handle_t));

  lcd->panel_handle = panel_handle;
  lcd->io_handle = io_handle;
  lcd->lcd_bl_gpio = config->lcd_bl;

  return lcd;
}

void lcd_set_orientation(st7796_lcd_handle_t *lcd, enum Orientation orientation) {
  switch (orientation) {
    case PORTRAIT:
      ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd->panel_handle, false));
      ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd->panel_handle, false, true));
      break;
    case LANDSCAPE:
      ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd->panel_handle, true));
      ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd->panel_handle, true, true));
      break;
    case INVERTED_PORTRAIT:
      ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd->panel_handle, false));
      ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd->panel_handle, true, false));
      break;
    case INVERTED_LANDSCAPE:
      ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd->panel_handle, true));
      ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd->panel_handle, false, false));
      break;
    default:
      printf("Error: Invalid orientation\n");
      break;
  }
}

void lcd_backlight_on(st7796_lcd_handle_t *lcd) {
  gpio_set_level(lcd->lcd_bl_gpio, 1);
}
void lcd_backlight_off(st7796_lcd_handle_t *lcd) {
  gpio_set_level(lcd->lcd_bl_gpio, 0);
}


