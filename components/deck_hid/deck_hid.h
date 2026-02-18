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

void deck_hid_init(void);
void deck_hid_send_state(deck_input_report_t *report);
