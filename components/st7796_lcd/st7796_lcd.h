#pragma once
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_st7796.h"

typedef struct {
  int lcd_host; 
  int spi_miso;
  int spi_mosi;
  int spi_sclk;
  int lcd_cs;
  int lcd_dc;
  int lcd_rst;
  int lcd_bl;
  int lcd_hor_res;
  int lcd_ver_res;
  int c_sda;
  int c_scl;
  int c_int;
  int c_rst;
} st7796_lcd_config_t;

typedef struct {
    esp_lcd_panel_handle_t panel_handle;
    esp_lcd_panel_io_handle_t io_handle;
    int lcd_bl_gpio;
} st7796_lcd_handle_t;

enum Orientation {
    PORTRAIT,
    LANDSCAPE,
    INVERTED_PORTRAIT,
    INVERTED_LANDSCAPE
};


st7796_lcd_handle_t* init_st7796_lcd(st7796_lcd_config_t *config);
void lcd_set_orientation(st7796_lcd_handle_t *lcd, enum Orientation orientation);
void lcd_backlight_on(st7796_lcd_handle_t *lcd);
void lcd_backlight_off(st7796_lcd_handle_t *lcd);

