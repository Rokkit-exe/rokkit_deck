#pragma once
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_types.h"

/* Waveshare 3.5" LCD HAT pinout
 * - SPI2_HOST (VSPI)
 * - MISO: 42 (not used by LCD, can be left unconnected)
 * - MOSI: 2
 * - SCLK: 1
 * - LCD CS: 39
 * - LCD DC: 41
 * - LCD RST: 40
 * - LCD BL: 5
 * - Touch SDA: 15
 * - Touch SCL: 7
 * - Touch INT: 17
 * - Touch RST: 16
 *
 * Note: The LCD and touch controller share the same SPI bus for data/commands,
 * but the touch controller uses I2C for communication with the ESP32. The MISO
 * pin is not used by the LCD but is required for SPI bus initialization.
 */
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
} bsp_config_t;

/* LCD orientation options
 * - PORTRAIT: 320x480, default orientation
 * - LANDSCAPE: 480x320, rotated 90° clockwise
 * - INVERTED_PORTRAIT: 320x480, rotated 180°
 * - INVERTED_LANDSCAPE: 480x320, rotated 90° counterclockwise
 */
enum Orientation { PORTRAIT, LANDSCAPE, INVERTED_PORTRAIT, INVERTED_LANDSCAPE };

typedef struct {
  esp_lcd_panel_io_handle_t lcd_io;
  esp_lcd_panel_handle_t lcd_panel;
  esp_lcd_panel_io_handle_t touch_io;
  esp_lcd_touch_handle_t touch_panel;
  uint16_t lcd_width;
  uint16_t lcd_height;
} bsp_handles_t;

esp_err_t bsp_init(bsp_config_t *config, bsp_handles_t *handles);
esp_err_t bsp_lcd_init(bsp_config_t *config, bsp_handles_t *handles);
esp_err_t bsp_touch_init(bsp_config_t *config, bsp_handles_t *handles);

/* Function to set the LCD orientation. This sends the appropriate command to
 * the LCD panel to change its orientation based on the provided enum value.
 * Parameters:
 * - panel_handle: Pointer to the handle of the initialized LCD panel.
 * - orientation: Enum value specifying the desired orientation (PORTRAIT,
 * LANDSCAPE, INVERTED_PORTRAIT, INVERTED_LANDSCAPE).
 */
void lcd_set_orientation(esp_lcd_panel_handle_t *panel_handle,
                         enum Orientation orientation);

/* Functions to control the LCD backlight. These functions set the specified
 * GPIO pin to turn the backlight on or off. Parameters:
 * - lcd_bl_gpio: The GPIO number connected to the LCD backlight control.
 */
void lcd_backlight_on(int lcd_bl_gpio);

/* Function to turn off the LCD backlight. This sets the specified GPIO pin to
 * turn the backlight off. Parameters:
 * - lcd_bl_gpio: The GPIO number connected to the LCD backlight control.
 */
void lcd_backlight_off(int lcd_bl_gpio);
