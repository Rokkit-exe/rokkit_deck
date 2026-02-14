#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_log.h"
#include "lvgl.h"

typedef struct {
  esp_lcd_touch_handle_t handle;
  uint16_t lcd_width;
  uint16_t lcd_height;
} touch_driver_ctx_t;

static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  touch_driver_ctx_t *ctx = lv_indev_get_user_data(indev);
  if (ctx->handle == NULL) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  esp_lcd_touch_read_data(ctx->handle);

  uint16_t touch_x[1];
  uint16_t touch_y[1];
  uint16_t touch_strength[1];
  uint8_t touch_count = 0;

  bool touched = esp_lcd_touch_get_coordinates(ctx->handle, touch_x, touch_y,
                                               touch_strength, &touch_count, 1);

  if (touched && touch_count > 0) {
    uint16_t raw_x = touch_x[0];
    uint16_t raw_y = touch_y[0];

    data->point.x = raw_y;
    data->point.y = ctx->lcd_height - raw_x;

    data->state = LV_INDEV_STATE_PRESSED;
    ESP_LOGI("TOUCH", "Raw: (%d, %d) -> Transformed: (%d, %d)", raw_x, raw_y,
             data->point.x, data->point.y);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area,
                          uint8_t *px_map) {
  esp_lcd_panel_handle_t panel =
      (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);

  esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1,
                            area->y2 + 1, px_map);

  lv_display_flush_ready(disp);
}

lv_indev_t *lvgl_create_touch(esp_lcd_touch_handle_t touch_handle,
                              uint16_t lcd_width, uint16_t lcd_height) {
  touch_driver_ctx_t *ctx = malloc(sizeof(touch_driver_ctx_t));
  ctx->handle = touch_handle;
  ctx->lcd_width = lcd_width;
  ctx->lcd_height = lcd_height;

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchpad_read);
  lv_indev_set_user_data(indev, ctx);

  return indev;
}

lv_display_t *lvgl_create_display(esp_lcd_panel_handle_t panel, uint16_t width,
                                  uint16_t height) {
  lv_display_t *disp = lv_display_create(width, height);
  lv_display_set_flush_cb(disp, lvgl_flush_cb);
  lv_display_set_user_data(disp, panel);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

  // Two smaller buffers for smoother rendering
  size_t buf_size = width * 30 * sizeof(lv_color_t);
  void *buf1 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
  void *buf2 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);

  lv_display_set_buffers(disp, buf1, buf2, buf_size,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_display_set_user_data(disp, panel);

  return disp;
}
