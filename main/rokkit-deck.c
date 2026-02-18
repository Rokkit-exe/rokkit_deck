#include "bsp_waveshare.h"
#include "deck_gl.h"
#include "deck_hid.h"
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
#include "lvgl_driver.h"
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

static bsp_config_t lcd_config = {.lcd_host = LCD_HOST,
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
static bsp_handles_t handles = {
    .lcd_io = NULL,
    .lcd_panel = NULL,
    .touch_io = NULL,
    .touch_panel = NULL,
    .lcd_width = LCD_HOR_RES,
    .lcd_height = LCD_VER_RES,
};

static void lvgl_tick_inc_cb(void *arg) { lv_tick_inc(10); }

static void lvgl_timer_task(void *arg) {
  ESP_LOGI("LVGL", "Timer task started");
  while (1) {
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void app_main(void) {

  esp_err_t err = bsp_init(&lcd_config, &handles);

  if (err != ESP_OK || handles.lcd_panel == NULL || handles.lcd_io == NULL) {
    ESP_LOGE("MAIN", "❌ LCD panel NOT initialized!");
    return;
  }
  ESP_LOGI("MAIN", "✓ LCD panel initialized");

  lcd_backlight_on(LCD_BL);
  vTaskDelay(pdMS_TO_TICKS(100));
  lcd_set_orientation(&handles.lcd_panel, INVERTED_LANDSCAPE);
  vTaskDelay(pdMS_TO_TICKS(100));

  lv_init();

  lvgl_create_display(handles.lcd_panel, LCD_HOR_RES, LCD_VER_RES);
  lvgl_create_touch(handles.touch_panel, LCD_HOR_RES, LCD_VER_RES);

  ESP_LOGI("MAIN", "✓ LVGL display and touch drivers initialized");
  deck_hid_init();
  ESP_LOGI("MAIN", "✓ HID device initialized");
  deck_create_ui();

  update_slider_value(0, 30);
  update_slider_value(1, 70);
  update_slider_value(2, 90);

  const esp_timer_create_args_t tick_timer_args = {
      .callback = &lvgl_tick_inc_cb,
      .name = "lvgl_tick",
      .dispatch_method = ESP_TIMER_TASK,
  };
  esp_timer_handle_t tick_timer;
  ESP_ERROR_CHECK(esp_timer_create(&tick_timer_args, &tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 10 * 1000)); // 10ms

  xTaskCreate(lvgl_timer_task, "lvgl", 6144, NULL, 4, NULL);

  ESP_LOGI("MAIN", "✓ System initialized");
}
