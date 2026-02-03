#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_st7796.h"
#include "lvgl.h"
#include "st7796_lcd.h"
#include "deck_gl.h"


#define LCD_HOST    SPI2_HOST
#define SPI_MISO 42
#define SPI_MOSI 2
#define SPI_SCLK 1
#define LCD_CS   14
#define LCD_DC   12
#define LCD_RST  11
#define LCD_BL   5
#define LCD_HOR_RES 480
#define LCD_VER_RES 320

#define C_SDA 15
#define C_SCL 7
#define C_INT 17
#define C_RST 16

void app_main(void) {
    st7796_lcd_config_t lcd_config = {
        .lcd_host = LCD_HOST,
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
        .c_rst = C_RST
    };

    st7796_lcd_handle_t* lcd = init_st7796_lcd(&lcd_config);

    lcd_set_orientation(lcd, LANDSCAPE);
    lcd_backlight_on(lcd);

    deck_config_t deck_cfg = {
        .io_handle = lcd->io_handle,
        .panel_handle = lcd->panel_handle,
        .hor_res = LCD_HOR_RES,
        .ver_res = LCD_VER_RES,
    };

    DeckGL* deck = init_deck_display(&deck_cfg);

    create_button_grid(deck);

    // 5. LVGL Timer Loop
    while (1) {
      // This function must be called periodically
      lv_timer_handler(); 
      vTaskDelay(pdMS_TO_TICKS(10));
    }
}
