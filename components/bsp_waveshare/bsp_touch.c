#include "bsp_waveshare.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_log.h"

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

static esp_err_t reset_gt911(bsp_config_t *config) {
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

esp_err_t bsp_touch_init(bsp_config_t *config, bsp_handles_t *handles) {
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

  err = esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, &handles->touch_io);
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

  err = esp_lcd_touch_new_i2c_gt911(handles->touch_io, &tp_cfg,
                                    &handles->touch_panel);

  if (err != ESP_OK) {
    ESP_LOGE("GT911", "Failed to initialize GT911 driver: %s",
             esp_err_to_name(err));
    esp_lcd_touch_del(handles->touch_panel);
    esp_lcd_panel_io_del(handles->touch_io);
    i2c_del_master_bus(i2c_bus);
    return err;
  } else {
    ESP_LOGI("GT911", "✓ GT911 driver initialized successfully");
  }

  return ESP_OK;
}
