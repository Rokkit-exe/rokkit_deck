#include "waveshare_lcd.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_st7796.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdlib.h"
#include <stdio.h>

esp_err_t init_st7796_panel(waveshare_lcd_config_t *config,
                            esp_lcd_panel_handle_t *out_panel,
                            esp_lcd_panel_io_handle_t *out_io_handle) {
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
                                 &io_config, out_io_handle);
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
  err = esp_lcd_new_panel_st7796(*out_io_handle, &panel_config, out_panel);
  err = esp_lcd_panel_reset(*out_panel);
  err = esp_lcd_panel_init(*out_panel);
  err = esp_lcd_panel_invert_color(*out_panel, true);
  err = esp_lcd_panel_disp_on_off(*out_panel, true);
  if (err != ESP_OK) {
    ESP_LOGE("LCD Panel", "Failed to initialize LCD panel: %s",
             esp_err_to_name(err));
    esp_lcd_panel_del(*out_panel);
    esp_lcd_panel_io_del(*out_io_handle);
    spi_bus_free(config->lcd_host);
    return err;
  }

  gpio_set_direction(config->lcd_bl, GPIO_MODE_OUTPUT);
  gpio_set_level(config->lcd_bl, 1);
  return ESP_OK;
}

static esp_err_t init_i2c_bus(int i2c_port, int sda_gpio, int scl_gpio,
                              int freq_hz, i2c_master_bus_handle_t *out_bus) {
  i2c_master_bus_config_t bus_cfg = {
      .i2c_port = i2c_port,
      .sda_io_num = sda_gpio,
      .scl_io_num = scl_gpio,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
  };
  esp_err_t err = i2c_new_master_bus(&bus_cfg, out_bus);
  if (err != ESP_OK) {
    return err;
  }
  return ESP_OK;
}

static esp_err_t reset_gt911(waveshare_lcd_config_t *config) {
  gpio_config_t io_conf = {
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = (1ULL << config->c_rst) | (1ULL << config->c_int),
      .pull_down_en = 0,
      .pull_up_en = 0,
  };
  esp_err_t err = gpio_config(&io_conf);
  if (err != ESP_OK) {
    return err;
  }

  gpio_set_level(config->c_int, 0);
  gpio_set_level(config->c_rst, 0);
  vTaskDelay(pdMS_TO_TICKS(11));

  gpio_set_level(config->c_int, 1);
  vTaskDelay(pdMS_TO_TICKS(1));

  gpio_set_level(config->c_rst, 1);
  vTaskDelay(pdMS_TO_TICKS(6));

  gpio_set_level(config->c_int, 0);
  vTaskDelay(pdMS_TO_TICKS(55));

  gpio_set_direction(config->c_int, GPIO_MODE_INPUT);
  vTaskDelay(pdMS_TO_TICKS(100));

  return ESP_OK;
}

esp_err_t init_gt911_panel(waveshare_lcd_config_t *config,
                           esp_lcd_panel_io_handle_t *tp_io_handle,
                           esp_lcd_touch_handle_t *tp_handle) {
  esp_err_t err = reset_gt911(config);
  if (err != ESP_OK) {
    ESP_LOGE("GT911", "Failed to reset GT911: %s", esp_err_to_name(err));
    return err;
  }

  i2c_master_bus_handle_t i2c_bus = NULL;
  err = init_i2c_bus(config->lcd_host, config->c_sda, config->c_scl, 400000,
                     &i2c_bus);
  if (err != ESP_OK) {
    ESP_LOGE("GT911", "Failed to initialize I2C bus: %s", esp_err_to_name(err));
    return err;
  }

  esp_lcd_panel_io_i2c_config_t tp_io_config =
      ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

  tp_io_config.scl_speed_hz = 400000;
  tp_io_config.dev_addr = 0x5D;

  err = esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, tp_io_handle);
  if (err != ESP_OK) {
    ESP_LOGE("GT911", "Failed to create touch panel IO: %s",
             esp_err_to_name(err));
    i2c_del_master_bus(i2c_bus);
    return err;
  }

  esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
      .dev_addr = tp_io_config.dev_addr,
  };

  esp_lcd_touch_config_t tp_cfg = {
      .x_max = config->lcd_hor_res,
      .y_max = config->lcd_ver_res,
      .rst_gpio_num = config->c_rst,
      .int_gpio_num = config->c_int,
      .levels =
          {
              .reset = 0,
              .interrupt = 0,
          },
      .flags =
          {
              .swap_xy = 0,
              .mirror_x = 0,
              .mirror_y = 0,
          },
      .driver_data = &tp_gt911_config,
  };

  err = esp_lcd_touch_new_i2c_gt911(*tp_io_handle, &tp_cfg, tp_handle);

  if (err != ESP_OK) {
    ESP_LOGE("GT911", "Failed to initialize GT911 driver: %s",
             esp_err_to_name(err));
    esp_lcd_touch_del(*tp_handle);
    esp_lcd_panel_io_del(*tp_io_handle);
    i2c_del_master_bus(i2c_bus);
    return err;
  } else {
    ESP_LOGI("GT911", "✓ GT911 driver initialized successfully");
  }

  return ESP_OK;
}

void lcd_set_orientation(esp_lcd_panel_handle_t *panel_handle,
                         enum Orientation orientation) {
  switch (orientation) {
  case PORTRAIT:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(*panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(*panel_handle, false, true));
    break;
  case LANDSCAPE:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(*panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(*panel_handle, true, true));
    break;
  case INVERTED_PORTRAIT:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(*panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(*panel_handle, true, false));
    break;
  case INVERTED_LANDSCAPE:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(*panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(*panel_handle, false, false));
    break;
  default:
    printf("Error: Invalid orientation\n");
    break;
  }
}

void lcd_backlight_on(int lcd_bl_gpio) { gpio_set_level(lcd_bl_gpio, 1); }
void lcd_backlight_off(int lcd_bl_gpio) { gpio_set_level(lcd_bl_gpio, 0); }
