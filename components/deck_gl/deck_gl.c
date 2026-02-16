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
#include "layouts/grid/lv_grid.h"
#include "lv_api_map_v8.h"
#include "lvgl.h"
#include "misc/lv_area.h"
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

ui_context_t ui_ctx;

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

static lv_obj_t *create_label(lv_obj_t *parent, label_t *cfg) {
  lv_obj_t *label = lv_label_create(parent);
  lv_label_set_text(label, cfg->label);
  lv_obj_set_style_text_color(label, cfg->text_color, 0);
  lv_obj_set_style_text_font(label, cfg->font, 0);
  return label;
}

static lv_obj_t *create_button(lv_obj_t *parent, button_t *cfg) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_style_bg_color(btn, cfg->bg_color, LV_PART_MAIN);
  lv_obj_set_style_radius(btn, cfg->radius, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_hex(BLACK), 0);

  lv_obj_t *label =
      create_label(btn, &(label_t){.label = cfg->label,
                                   .text_color = lv_color_hex(WHITE),
                                   .font = &lv_font_montserrat_14});

  ui_ctx.btn_labels[cfg->id - 1] = label;
  lv_obj_center(label);
  lv_obj_add_event_cb(btn, grid_button_pressed_event_cb, LV_EVENT_PRESSED,
                      (void *)cfg->id);
  lv_obj_add_event_cb(btn, grid_button_release_event_cb, LV_EVENT_RELEASED,
                      (void *)cfg->id);
  return btn;
}

static lv_obj_t *create_container(lv_obj_t *parent, container_t *cfg) {
  lv_obj_t *container = lv_obj_create(parent);
  lv_obj_set_size(container, cfg->width, cfg->height);
  lv_obj_align(container, cfg->align, 0, 0);
  lv_obj_set_style_bg_color(container, cfg->bg_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(container, 0, 0);
  lv_obj_set_style_pad_all(container, cfg->padding, 0);
  return container;
}

static void create_grid(lv_obj_t *obj) {
  // set grid layout
  lv_obj_set_layout(obj, LV_LAYOUT_GRID);

  static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                              LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                              LV_GRID_TEMPLATE_LAST};

  lv_obj_set_grid_dsc_array(obj, col_dsc, row_dsc);
  lv_obj_set_style_pad_column(obj, 5, 0);
  lv_obj_set_style_pad_row(obj, 5, 0);
}

static void create_button_grid(lv_obj_t *scr) {
  // control Create button grid container
  lv_obj_t *btn_container =
      create_container(scr, &(container_t){.width = 480,
                                           .height = 220,
                                           .align = LV_ALIGN_TOP_MID,
                                           .padding = 10,
                                           .bg_color = lv_color_hex(BLACK)});

  create_grid(btn_container);

  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 4; col++) {
      int btn_id = row * 4 + col + 1;

      char btn_text[8];
      snprintf(btn_text, sizeof(btn_text), "Btn %d", btn_id);
      lv_obj_t *btn = create_button(btn_container,
                                    &(button_t){.id = btn_id,
                                                .label = btn_text,
                                                .bg_color = lv_color_hex(BLUE),
                                                .radius = 8});
      lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, col, 1,
                           LV_GRID_ALIGN_STRETCH, row, 1);
      ui_ctx.btn[btn_id - 1] = btn;
    }
  }
}

static lv_obj_t *create_slider(lv_obj_t *parent, slider_t *cfg) {
  lv_obj_t *slider = lv_slider_create(parent);
  lv_obj_set_width(slider, cfg->width);
  lv_slider_set_range(slider, cfg->min, cfg->max);
  lv_slider_set_value(slider, cfg->value, LV_ANIM_OFF);

  lv_obj_set_style_bg_color(slider, cfg->main_color, LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, cfg->indicator_color, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, cfg->knob_color, LV_PART_KNOB);
  lv_obj_set_style_radius(slider, 5, LV_PART_MAIN);

  return slider;
}

static void create_slider_grid(lv_obj_t *scr) {
  lv_obj_t *slider_container =
      create_container(scr, &(container_t){.width = 480,
                                           .height = 100,
                                           .align = LV_ALIGN_BOTTOM_MID,
                                           .padding = 10,
                                           .bg_color = lv_color_hex(BLACK)});

  lv_obj_set_flex_flow(slider_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(slider_container, LV_FLEX_ALIGN_SPACE_EVENLY,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  const char *slider_names[] = {"VOL", "BRT", "SPD"};

  for (int i = 0; i < 3; i++) {
    // Container for each slider + label
    lv_obj_t *slider_box = create_container(
        slider_container, &(container_t){.width = 130,
                                         .height = 80,
                                         .align = LV_ALIGN_CENTER,
                                         .padding = 5,
                                         .bg_color = lv_color_hex(BLACK),
                                         .border_width = 0,
                                         .radius = 8});

    lv_obj_set_flex_flow(slider_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(slider_box, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *name_label =
        create_label(slider_box, &(label_t){.label = slider_names[i],
                                            .text_color = lv_color_hex(WHITE),
                                            .font = &lv_font_montserrat_14});
    ui_ctx.slider_name_labels[i] = name_label;
    lv_obj_t *slider = create_slider(
        slider_box, &(slider_t){.width = 100,
                                .min = 0,
                                .max = 100,
                                .value = 50,
                                .main_color = lv_color_hex(BLUE),
                                .indicator_color = lv_color_hex(RED),
                                .knob_color = lv_color_hex(GREEN)});

    lv_obj_t *value_label =
        create_label(slider_box, &(label_t){.label = "50",
                                            .text_color = lv_color_hex(WHITE),
                                            .font = &lv_font_montserrat_14});
    ui_ctx.slider_value_labels[i] = value_label;
    ui_ctx.sliders[i] = slider;
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED,
                        value_label);
  }
}

void update_slider_value(int slider_index, int value) {
  lv_slider_set_value(ui_ctx.sliders[slider_index], value, LV_ANIM_OFF);
  lv_label_set_text_fmt(ui_ctx.slider_value_labels[slider_index], "%d", value);
}

void update_slider_text(int slider_index, const char *label) {
  lv_label_set_text(ui_ctx.slider_name_labels[slider_index], label);
}

void update_button_color(int btn_index, lv_color_t color) {
  lv_obj_set_style_bg_color(ui_ctx.btn[btn_index], color, LV_PART_MAIN);
  lv_obj_invalidate(ui_ctx.btn[btn_index]);
}

void update_button_text(int btn_index, const char *label) {
  lv_label_set_text(ui_ctx.btn_labels[btn_index], label);
  lv_obj_invalidate(ui_ctx.btn_labels[btn_index]);
}

void deck_create_ui(void) {
  lv_obj_t *scr = lv_screen_active();

  lv_obj_set_style_bg_color(scr, lv_color_hex(BLACK), LV_PART_MAIN);

  create_button_grid(scr);
  create_slider_grid(scr);
}
