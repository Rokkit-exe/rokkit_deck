#include "display/lv_display.h"
#include "layouts/flex/lv_flex.h"
#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "deck_gl.h"

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
    int x1 = area->x1;
    int y1 = area->y1;
    int x2 = area->x2;
    int y2 = area->y2;
    // Copy the internal LVGL buffer to the LCD
    esp_lcd_panel_draw_bitmap(panel_handle, x1, y1, x2 + 1, y2 + 1, px_map);
}

DeckGL* init_deck_display(deck_config_t *cfg) {
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    // Initialize deck display settings
    // 2. Initialize LVGL
    lv_init();
    lv_display_t *disp = lv_display_create(cfg->hor_res, cfg->ver_res);
    
    // 3. Allocate draw buffers (Recommend putting these in internal RAM for speed, or PSRAM for size)
    size_t buffer_size = cfg->hor_res * 50 * sizeof(lv_color16_t);
    void *buf1 = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    
    lv_display_set_buffers(disp, buf1, NULL, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(disp, cfg->panel_handle);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
    esp_lcd_panel_io_register_event_callbacks(cfg->io_handle, &cbs, disp);

    DeckGL* deck = calloc(1, sizeof(DeckGL));
    deck->buffer_size = buffer_size;
    deck->draw_buffer = buf1;
    deck->display = disp;
    deck->hor_res = cfg->hor_res;
    deck->ver_res = cfg->ver_res;
    return deck;
}

void create_button_grid(DeckGL* deck) {
    lv_obj_t * scr = lv_display_get_screen_active(deck->display);

    // --- 1. Button Grid Container (75% height) ---
    lv_obj_t * container = lv_obj_create(scr);
    lv_obj_set_size(container, lv_pct(100), lv_pct(75));
    lv_obj_align(container, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_obj_set_style_pad_all(container, 10, 0); 
    lv_obj_set_style_pad_gap(container, 5, 0);

    for(int i = 0; i < 8; i++) {
        lv_obj_t * btn = lv_button_create(container);
        lv_obj_set_size(btn, lv_pct(23), lv_pct(38)); 
        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "%d", i + 1);
        lv_obj_center(label);
    }

    // --- 2. Slider Container (Remaining 25% height) ---
    lv_obj_t * slider_cont = lv_obj_create(scr);
    lv_obj_set_size(slider_cont, lv_pct(100), lv_pct(25));
    lv_obj_align(slider_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    // Remove border and background to make it look like a seamless "tray"
    lv_obj_set_style_border_width(slider_cont, 0, 0);
    lv_obj_set_style_bg_opa(slider_cont, LV_OPA_TRANSP, 0);

    // Flexbox for Sliders: Single row, evenly spaced
    lv_obj_set_flex_flow(slider_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(slider_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create 3 Sliders
    for(int i = 0; i < 3; i++) {
        lv_obj_t * slider = lv_slider_create(slider_cont);
        lv_obj_set_size(slider, lv_pct(25), 10); // 25% width of the tray, 10px height
        lv_slider_set_value(slider, 50, LV_ANIM_OFF); // Start at middle
    }
}


