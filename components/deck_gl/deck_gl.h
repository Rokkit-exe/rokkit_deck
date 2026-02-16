#pragma once

#include "esp_lcd_panel_io.h"
#include "lvgl.h"
#include <stdint.h>

/* label configuration
 * - const char *label
 * - lv_color_t text_color
 * - const lv_font_t *font
 */
typedef struct {
  const char *label;
  lv_color_t text_color;
  const lv_font_t *font;
} label_t;

/* container configuration
 * - uint32_t width
 * - uint32_t height
 * - lv_align_t align
 * - uint32_t padding
 * - lv_color_t bg_color
 * - uint32_t border_width
 * - uint32_t radius
 */
typedef struct {
  uint32_t width;
  uint32_t height;
  lv_align_t align;
  uint32_t padding;
  lv_color_t bg_color;
  uint32_t border_width;
  uint32_t radius;
} container_t;

/* button configuration
 * - uint32_t id
 * - const char *label
 * - lv_color_t bg_color
 * - uint32_t radius
 */
typedef struct {
  uint32_t id;
  const char *label;
  lv_color_t bg_color;
  uint32_t radius;
} button_t;

/* slider configuration
 * - const char *label
 * - uint32_t width
 * - uint32_t min
 * - uint32_t max
 * - uint32_t value
 * - lv_color_t main_color
 * - lv_color_t indicator_color
 * - lv_color_t knob_color
 * - lv_color_t text_color
 */
typedef struct {
  uint32_t width;
  uint32_t min;
  uint32_t max;
  uint32_t value;
  lv_color_t main_color;
  lv_color_t indicator_color;
  lv_color_t knob_color;
  lv_color_t text_color;
} slider_t;

/* UI configuration to hold settings for all UI elements
 * - btn_configs[8]: Array of button configurations for 8 buttons
 * - slider_configs[3]: Array of slider configurations for 3 sliders
 * - slider_names[3]: Array of names for the sliders
 * - font: Font to be used for all text elements
 */
typedef struct {
  button_t btn_configs[8];
  slider_t slider_configs[3];
  char *slider_names[3];
  const lv_font_t *font;
} ui_config_t;

/* UI context to hold references to created objects for later use (e.g. event
 * handling)
 * - btn[8]: Array of button objects
 * - btn_labels[8]: Array of button label objects
 * - sliders[3]: Array of slider objects
 * - slider_name_labels[3]: Array of slider name label objects
 * - slider_value_labels[3]: Array of slider value label objects
 */
typedef struct {
  lv_obj_t *btn[8];
  lv_obj_t *btn_labels[8];
  lv_obj_t *sliders[3];
  lv_obj_t *slider_name_labels[3];
  lv_obj_t *slider_value_labels[3];
} ui_context_t;

void deck_create_ui(void);
void update_slider_value(int slider_index, int value);
void update_slider_text(int slider_index, const char *label);
void update_button_color(int btn_index, lv_color_t color);
void update_button_text(int btn_index, const char *label);
