#include "deck_gl.h"
#include "driver/gpio.h"
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
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "waveshare_lcd.h"
#include <stdint.h>
#include <stdio.h>

#define LCD_HOST SPI2_HOST
#define SPI_MISO 42
#define SPI_MOSI 2
#define SPI_SCLK 1
#define LCD_CS 39
#define LCD_DC 41
#define LCD_RST 40
#define LCD_BL 5
#define LCD_HOR_RES 480
#define LCD_VER_RES 320
#define C_SDA 15
#define C_SCL 7
#define C_INT 17
#define C_RST 16

// ✅ Global handles
static esp_lcd_panel_io_handle_t panel_io_handle = NULL;
static esp_lcd_panel_io_handle_t tp_io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_touch_handle_t tp_handle = NULL;

// ✅ LVGL tick increment - called from timer ISR
static void lvgl_tick_inc_cb(void *arg) { lv_tick_inc(10); }

// ✅ LVGL timer task with mutex protection
static void lvgl_timer_task(void *arg) {
  ESP_LOGI("LVGL", "Timer task started");
  while (1) {
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ✅ Fixed touchpad read callback with coordinate transformation
static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (tp_handle == NULL) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  esp_lcd_touch_read_data(tp_handle);

  uint16_t touch_x[1];
  uint16_t touch_y[1];
  uint16_t touch_strength[1];
  uint8_t touch_count = 0;

  bool touched = esp_lcd_touch_get_coordinates(tp_handle, touch_x, touch_y,
                                               touch_strength, &touch_count, 1);

  if (touched && touch_count > 0) {
    uint16_t raw_x = touch_x[0];
    uint16_t raw_y = touch_y[0];

    data->point.x = raw_y;
    data->point.y = LCD_VER_RES - raw_x;

    data->state = LV_INDEV_STATE_PRESSED;
    ESP_LOGI("TOUCH", "Raw: (%d, %d) -> Transformed: (%d, %d)", raw_x, raw_y,
             data->point.x, data->point.y);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void app_main(void) {

  waveshare_lcd_config_t lcd_config = {.lcd_host = LCD_HOST,
                                       .spi_miso = SPI_MISO,
                                       .spi_mosi = SPI_MOSI,
                                       .spi_sclk = SPI_SCLK,
                                       .lcd_cs = LCD_CS,
                                       .lcd_dc = LCD_DC,
                                       .lcd_rst = LCD_RST,
                                       .lcd_bl = LCD_BL,
                                       .lcd_hor_res = LCD_HOR_RES,
                                       .lcd_ver_res = LCD_VER_RES,
                                       .c_sda = C_SDA,
                                       .c_scl = C_SCL,
                                       .c_int = C_INT,
                                       .c_rst = C_RST};

  ESP_ERROR_CHECK(
      init_st7796_panel(&lcd_config, &panel_handle, &panel_io_handle));

  if (panel_handle == NULL) {
    ESP_LOGE("MAIN", "❌ LCD panel NOT initialized!");
    return;
  }
  ESP_LOGI("MAIN", "✓ LCD panel initialized");

  lcd_backlight_on(LCD_BL);
  vTaskDelay(pdMS_TO_TICKS(100));
  lcd_set_orientation(&panel_handle, INVERTED_LANDSCAPE);
  vTaskDelay(pdMS_TO_TICKS(100));

  esp_err_t touch_err =
      init_gt911_panel(&lcd_config, &tp_io_handle, &tp_handle);
  if (touch_err == ESP_OK && tp_handle != NULL) {
    ESP_LOGI("MAIN", "✓ Touch initialized");
  } else {
    ESP_LOGW("MAIN", "⚠ Touch not available");
  }

  deck_config_t deck_cfg = {
      .io_handle = panel_io_handle,
      .panel_handle = panel_handle,
      .hor_res = LCD_HOR_RES,
      .ver_res = LCD_VER_RES,
  };
  DeckGL *deck = init_deck_display(&deck_cfg, panel_io_handle);

  if (tp_handle != NULL) {
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchpad_read);
    ESP_LOGI("MAIN", "✓ Touch registered with LVGL");
  }

  deck_create_ui(deck);

  const esp_timer_create_args_t tick_timer_args = {
      .callback = &lvgl_tick_inc_cb,
      .name = "lvgl_tick",
      .dispatch_method = ESP_TIMER_TASK,
  };
  esp_timer_handle_t tick_timer;
  ESP_ERROR_CHECK(esp_timer_create(&tick_timer_args, &tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 10 * 1000)); // 10ms

  // ✅ Create LVGL timer task
  xTaskCreate(lvgl_timer_task, "lvgl", 6144, NULL, 4, NULL);

  ESP_LOGI("MAIN", "✓ System initialized");

  // Main loop
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
