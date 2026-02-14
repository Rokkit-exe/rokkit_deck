#include "bsp_waveshare.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7796.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_log.h"

esp_err_t bsp_lcd_init(bsp_config_t *config, bsp_handles_t *handles) {
  spi_bus_config_t buscfg = {
      .sclk_io_num = config->spi_sclk,
      .mosi_io_num = config->spi_mosi,
      .miso_io_num = -1,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = config->lcd_hor_res * 80 * sizeof(uint16_t),

  };
  esp_err_t err =
      spi_bus_initialize(config->lcd_host, &buscfg, SPI_DMA_CH_AUTO);
  if (err != ESP_OK) {
    ESP_LOGE("SPI", "Failed to initialize SPI bus: %s", esp_err_to_name(err));
    return err;
  }

  esp_lcd_panel_io_spi_config_t io_config = {
      .dc_gpio_num = config->lcd_dc,
      .cs_gpio_num = config->lcd_cs,
      .pclk_hz = 40 * 1000 * 1000,
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
      .spi_mode = 0,
      .trans_queue_depth = 10,
  };
  err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)config->lcd_host,
                                 &io_config, &handles->lcd_io);
  if (err != ESP_OK) {
    ESP_LOGE("LCD IO", "Failed to create LCD panel IO: %s",
             esp_err_to_name(err));
    spi_bus_free(config->lcd_host);
    return err;
  }

  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = config->lcd_rst,
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
      .bits_per_pixel = 16,
  };
  err = esp_lcd_new_panel_st7796(handles->lcd_io, &panel_config,
                                 &handles->lcd_panel);
  err = esp_lcd_panel_reset(handles->lcd_panel);
  err = esp_lcd_panel_init(handles->lcd_panel);
  err = esp_lcd_panel_invert_color(handles->lcd_panel, true);
  err = esp_lcd_panel_disp_on_off(handles->lcd_panel, true);
  if (err != ESP_OK) {
    ESP_LOGE("LCD Panel", "Failed to initialize LCD panel: %s",
             esp_err_to_name(err));
    esp_lcd_panel_del(handles->lcd_panel);
    esp_lcd_panel_io_del(handles->lcd_io);
    spi_bus_free(config->lcd_host);
    return err;
  }

  gpio_set_direction(config->lcd_bl, GPIO_MODE_OUTPUT);
  gpio_set_level(config->lcd_bl, 1);
  return ESP_OK;
}
