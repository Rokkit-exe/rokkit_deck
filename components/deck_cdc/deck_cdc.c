#include "deck_cdc.h"
#include "class/cdc/cdc_device.h"
#include "common/tusb_types.h"
#include "deck_cdc_desc.h"
#include "deck_gl.h"
#include "device/usbd.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_private/usb_phy.h"
#include "hal/usb_phy_types.h"
#include "hal/usb_serial_jtag_ll.h"
#include "soc/usb_serial_jtag_reg.h"
#include "sys/unistd.h"
#include "tinyusb_default_config.h"
#include "tusb.h"
#include <stdint.h>

static void device_event_handler(tinyusb_event_t *event, void *arg) {
  switch (event->id) {
  case TINYUSB_EVENT_ATTACHED:
    ESP_LOGI("USB", "Device attached");
    break;
  case TINYUSB_EVENT_DETACHED:
    ESP_LOGI("USB", "Device detached");
    break;
  default:
    ESP_LOGW("USB", "Unknown USB event: %d", event->id);
    break;
  }
}

static const char lang_descriptor[] = {0x09, 0x04};
static const char *descriptor_strings[] = {
    [0] = lang_descriptor, // static, not a compound literal
    [1] = "Freenove ESP32-S3", [2] = "ESP32 Stream Deck",
    [3] = "1234567890AB",      [4] = NULL,
};

void deck_usb_init(void) {
  // Claim PHY once
  usb_phy_config_t phy_config = {
      .controller = USB_PHY_CTRL_OTG,
      .target = USB_PHY_TARGET_INT,
      .otg_mode = USB_OTG_MODE_DEVICE,
      .otg_speed = USB_PHY_SPEED_FULL,
  };
  usb_phy_handle_t phy_handle;
  ESP_ERROR_CHECK(usb_new_phy(&phy_config, &phy_handle));

  tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG(device_event_handler);
  tusb_cfg.port = 0;
  tusb_cfg.descriptor.device = &device_desc;
  tusb_cfg.descriptor.string = descriptor_strings;
  tusb_cfg.descriptor.string_count = 5;
  tusb_cfg.descriptor.full_speed_config =
      composite_config_descriptor; // must include both CDC and HID interfaces
  tusb_cfg.phy.skip_setup = true;

  ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
}

#define CDC_BUF_SIZE 64
#define CDC_RX_BUFFER_SIZE 256

static uint8_t rx_buffer[CDC_RX_BUFFER_SIZE];
static uint16_t rx_index = 0;
static uint16_t expected_length = 0;

void process_complete_packet(uint8_t *packet, uint16_t total_len) {
  uint8_t report_id = packet[0];
  uint8_t version = packet[1];
  uint16_t payload_len = packet[2] | (packet[3] << 8);

  ESP_LOGI("CDC",
           "report_id=0x%02x version=0x%02x payload_len=%d sizeof(cfg)=%d",
           report_id, version, payload_len, sizeof(user_config_report_t));
  if (report_id != 0x10)
    return;
  if (version != 0x01)
    return;

  if (payload_len != sizeof(user_config_report_t))
    return;

  user_config_report_t cfg;
  memcpy(&cfg, &packet[4], sizeof(cfg));

  update_ui(&cfg);

  ESP_LOGI("CDC", "Config applied");

  send_ack();
  tud_cdc_write_flush();
}

void tud_cdc_rx_cb(uint8_t itf) {
  (void)itf;

  while (tud_cdc_available()) { // drain the entire FIFO
    uint8_t temp[64];
    ESP_LOGI("CDC", "Data available, reading...");
    uint32_t count = tud_cdc_read(temp, sizeof(temp));

    for (uint32_t i = 0; i < count; i++) {
      if (rx_index < CDC_RX_BUFFER_SIZE) {
        rx_buffer[rx_index++] = temp[i];
      }

      if (rx_index == 4) {
        uint16_t payload_len = rx_buffer[2] | (rx_buffer[3] << 8);
        expected_length = 4 + payload_len;
      }

      if (expected_length > 0 && rx_index >= expected_length) {
        ESP_LOGI("CDC", "Complete packet received, processing...");
        process_complete_packet(rx_buffer, expected_length);
        rx_index = 0;
        expected_length = 0;
      }
    }
  }
}

void usb_device_task(void *params) {
  while (1) {
    tud_task();  // call as fast as possible
    taskYIELD(); // yield but don't sleep
  }
}

// Optional: called when DTR/RTS changes
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  ESP_LOGI("CDC", "CDC line state changed: DTR=%d RTS=%d", dtr, rts);
}

// Send data back to host via CDC
void deck_cdc_write(const uint8_t *data, uint32_t len) {
  if (!tud_cdc_connected())
    return;

  uint32_t sent = 0;
  while (sent < len) {
    uint32_t chunk = len - sent;
    if (chunk > CDC_BUF_SIZE)
      chunk = CDC_BUF_SIZE;

    uint32_t written = tud_cdc_write(data + sent, chunk);
    tud_cdc_write_flush(); // flush buffer to host
    sent += written;
  }
}

void send_ack(void) {
  const uint8_t ack[4] = {
      0x10,      // report ID for ACK
      0x01,      // version
      0x00, 0x00 // payload length = 0
  };
  deck_cdc_write(ack, sizeof(ack));
}

void send_input_report(deck_input_report_t *report) {
  uint8_t buf[8];
  buf[0] = 0x01; // report ID for input report
  buf[1] = 0x01; // version
  buf[2] = sizeof(deck_input_report_t) & 0xFF;
  buf[3] = sizeof(deck_input_report_t) >> 8;
  memcpy(buf + 4, report, sizeof(deck_input_report_t));
  deck_cdc_write(buf, sizeof(buf));
}
