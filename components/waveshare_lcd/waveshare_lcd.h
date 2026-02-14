#pragma once
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

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
} waveshare_lcd_config_t;

/* LCD orientation options
 * - PORTRAIT: 320x480, default orientation
 * - LANDSCAPE: 480x320, rotated 90° clockwise
 * - INVERTED_PORTRAIT: 320x480, rotated 180°
 * - INVERTED_LANDSCAPE: 480x320, rotated 90° counterclockwise
 */
enum Orientation { PORTRAIT, LANDSCAPE, INVERTED_PORTRAIT, INVERTED_LANDSCAPE };

/* Initialization function for the ST7796 LCD panel. This sets up the SPI bus,
 * configures the LCD panel IO, and initializes the LCD panel itself. It returns
 * ESP_OK on success or an error code on failure. Parameters:
 * - config: Pointer to a waveshare_lcd_config_t struct containing the pin
 * configuration and display resolution.
 * - out_panel: Output parameter that will hold the handle to the initialized
 * LCD panel.
 * - out_io_handle: Output parameter that will hold the handle to the LCD panel
 * IO.
 */
esp_err_t init_st7796_panel(waveshare_lcd_config_t *config,
                            esp_lcd_panel_handle_t *out_panel,
                            esp_lcd_panel_io_handle_t *out_io_handle);

/* Initialization function for the GT911 touch controller.
 * This sets up the I2C bus, configures the touch panel IO,
 * and initializes the touch panel itself. It returns ESP_OK on success or an
 * error code on failure. Parameters:
 * - config: Pointer to a waveshare_lcd_config_t struct containing the pin
 * configuration and display resolution.
 * - tp_io_handle: Output parameter that will hold the handle to the touch panel
 * IO.
 * - tp_handle: Output parameter that will hold the handle to the initialized
 * touch panel.
 */
esp_err_t init_gt911_panel(waveshare_lcd_config_t *config,
                           esp_lcd_panel_io_handle_t *tp_io_handle,
                           esp_lcd_touch_handle_t *tp_handle);

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
