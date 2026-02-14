#include "bsp_waveshare.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_types.h"
#include "esp_log.h"

esp_err_t bsp_init(bsp_config_t *config, bsp_handles_t *handles) {
  esp_err_t err = bsp_lcd_init(config, handles);
  if (err != ESP_OK) {
    ESP_LOGE("BSP", "Failed to initialize LCD panel: %s", esp_err_to_name(err));
    return err;
  }
  ESP_LOGI("BSP", "✓ LCD panel initialized successfully");

  err = bsp_touch_init(config, handles);
  if (err != ESP_OK) {
    ESP_LOGW("BSP", "⚠ Touch panel not initialized: %s", esp_err_to_name(err));
    handles->touch_panel = NULL; // Ensure touch panel handle is NULL on failure
  } else {
    ESP_LOGI("BSP", "✓ Touch panel initialized successfully");
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
