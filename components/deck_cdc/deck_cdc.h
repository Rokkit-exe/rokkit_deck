#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) {
  // Report ID 1 — sent automatically by deck_hid_send_state
  // 8 buttons packed into 1 byte (bit 0 = button 1, bit 7 = button 8)
  union {
    uint8_t buttons;
    struct {
      uint8_t btn1 : 1;
      uint8_t btn2 : 1;
      uint8_t btn3 : 1;
      uint8_t btn4 : 1;
      uint8_t btn5 : 1;
      uint8_t btn6 : 1;
      uint8_t btn7 : 1;
      uint8_t btn8 : 1;
    };
  };
  // 3 sliders, 0–100
  uint8_t slider1;
  uint8_t slider2;
  uint8_t slider3;
} deck_input_report_t;

#define MAX_BUTTON_LABEL_LEN 16
#define MAX_SLIDER_LABEL_LEN 16

typedef struct __attribute__((packed)) {
  char label[MAX_BUTTON_LABEL_LEN]; // null-terminated
  uint8_t bg_color[3];              // RGB
} button_config_t;

typedef struct __attribute__((packed)) {
  char label[MAX_SLIDER_LABEL_LEN]; // null-terminated
  uint8_t fg_color[3];
  uint8_t value;
} slider_config_t;

typedef struct __attribute__((packed)) {
  button_config_t buttons[8];
  slider_config_t sliders[3];
} user_config_report_t;

void deck_usb_init(void);
void deck_cdc_read_task(void *params);
void send_deck_input(deck_input_report_t *report);
void send_ack(void);
void send_input_report(deck_input_report_t *report);
void usb_device_task(void *params);
