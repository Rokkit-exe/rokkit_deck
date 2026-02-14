#include "deck_gl.h"
#include "core/lv_obj_style.h"
#include "display/lv_display.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_types.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "font/lv_font.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "layouts/flex/lv_flex.h"
#include "lv_api_map_v8.h"
#include "lvgl.h"
#include "misc/lv_color.h"
#include "misc/lv_event.h"
#include "misc/lv_types.h"
#include "tick/lv_tick.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BLACK 0x000000
#define WHITE 0xFFFFFF
#define BLUE 0xFF0000
#define RED 0x00FF00
#define GREEN 0x0000FF

// Button click handler
static void grid_button_pressed_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  int btn_id = (int)lv_event_get_user_data(e);
  ESP_LOGI("GRID", "Button %d pressed", btn_id);

  // Visual feedback - flash the button
  lv_obj_set_style_bg_color(btn, lv_color_hex(GREEN), LV_PART_MAIN);
}

static void grid_button_release_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  int btn_id = (int)lv_event_get_user_data(e);
  ESP_LOGI("GRID", "Button %d released", btn_id);

  // Visual feedback - flash the button
  lv_obj_set_style_bg_color(btn, lv_color_hex(BLUE), LV_PART_MAIN);
  lv_obj_invalidate(btn);
}

// Slider event handler
static void slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
  int value = lv_slider_get_value(slider);

  lv_label_set_text_fmt(label, "%d", value);
  ESP_LOGI("SLIDER", "Value: %d", value);
}

static void create_button_grid(lv_obj_t *scr) {
  // control Create button grid container
  lv_obj_t *btn_container = lv_obj_create(scr);
  lv_obj_set_size(btn_container, 480, 200); // 4 columns x 2 rows
  lv_obj_align(btn_container, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(btn_container, lv_color_hex(BLACK), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn_container, 0, 0);
  lv_obj_set_style_pad_all(btn_container, 5, 0);

  // set grid layout
  lv_obj_set_layout(btn_container, LV_LAYOUT_GRID);

  static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                              LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                              LV_GRID_TEMPLATE_LAST};

  lv_obj_set_grid_dsc_array(btn_container, col_dsc, row_dsc);
  lv_obj_set_style_pad_column(btn_container, 5, 0);
  lv_obj_set_style_pad_row(btn_container, 5, 0);

  // Create 4x2 grid of buttons
  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 4; col++) {
      int btn_id = row * 4 + col + 1;

      lv_obj_t *btn = lv_button_create(btn_container);
      lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, col, 1,
                           LV_GRID_ALIGN_STRETCH, row, 1);

      lv_obj_set_style_bg_color(btn, lv_color_hex(BLUE), LV_PART_MAIN);
      lv_obj_set_style_radius(btn, 8, 0);
      lv_obj_set_style_shadow_width(btn, 5, 0);
      lv_obj_set_style_shadow_color(btn, lv_color_hex(BLACK), 0);

      lv_obj_t *label = lv_label_create(btn);
      lv_label_set_text_fmt(label, "BTN %d", btn_id);
      lv_obj_center(label);
      lv_obj_set_style_text_color(label, lv_color_hex(WHITE), 0);
      lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

      lv_obj_add_event_cb(btn, grid_button_pressed_event_cb, LV_EVENT_PRESSED,
                          (void *)btn_id);
      lv_obj_add_event_cb(btn, grid_button_release_event_cb, LV_EVENT_RELEASED,
                          (void *)btn_id);
    }
  }
}

static void create_slider_grid(lv_obj_t *scr) {
  lv_obj_t *slider_container = lv_obj_create(scr);
  lv_obj_set_size(slider_container, 480, 100);
  lv_obj_align(slider_container, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_color(slider_container, lv_color_hex(BLACK),
                            LV_PART_MAIN);
  lv_obj_set_style_border_width(slider_container, 0, 0);
  lv_obj_set_style_pad_all(slider_container, 10, 0);

  lv_obj_set_flex_flow(slider_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(slider_container, LV_FLEX_ALIGN_SPACE_EVENLY,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  const char *slider_names[] = {"VOL", "BRT", "SPD"};

  for (int i = 0; i < 3; i++) {
    // Container for each slider + label
    lv_obj_t *slider_box = lv_obj_create(slider_container);
    lv_obj_set_size(slider_box, 130, 80);
    lv_obj_set_style_bg_color(slider_box, lv_color_hex(BLACK), LV_PART_MAIN);
    lv_obj_set_style_border_width(slider_box, 0, 0);
    lv_obj_set_style_radius(slider_box, 8, 0);
    lv_obj_set_style_pad_all(slider_box, 5, 0);

    lv_obj_set_flex_flow(slider_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(slider_box, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *name_label = lv_label_create(slider_box);
    lv_label_set_text(name_label, slider_names[i]);
    lv_obj_set_style_text_color(name_label, lv_color_hex(WHITE), 0);

    lv_obj_t *slider = lv_slider_create(slider_box);
    lv_obj_set_width(slider, 100);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(slider, lv_color_hex(BLUE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(BLUE), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(WHITE), LV_PART_KNOB);
    lv_obj_set_style_radius(slider, 5, LV_PART_MAIN);

    lv_obj_t *value_label = lv_label_create(slider_box);
    lv_label_set_text(value_label, "50");
    lv_obj_set_style_text_color(value_label, lv_color_hex(WHITE), 0);

    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED,
                        value_label);
  }
}

void deck_create_ui(void) {
  lv_obj_t *scr = lv_screen_active();

  lv_obj_set_style_bg_color(scr, lv_color_hex(BLACK), LV_PART_MAIN);

  create_button_grid(scr);
  create_slider_grid(scr);
}
