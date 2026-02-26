#include "deck_hid.h"
#include "common/tusb_types.h"
#include "deck_hid_desc.h"
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
  tusb_cfg.descriptor.string_count = 4;
  tusb_cfg.descriptor.full_speed_config =
      composite_config_descriptor; // must include both CDC and HID interfaces
  tusb_cfg.phy.skip_setup = true;

  ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
}

const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance) {
  return deck_hid_report_descriptor; // your HID report descriptor
}

// Called when host requests a report (GET_REPORT)
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)reqlen;
  // Fill buffer with current state if needed
  buffer[0] = 0; // buttons
  buffer[1] = 0; // s1
  buffer[2] = 0; // s2
  buffer[3] = 0; // s3
  return 4;
}

// Called when host sends a report (SET_REPORT / OUTPUT report)
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, const uint8_t *buffer,
                           uint16_t bufsize) {
  (void)instance;
  ESP_LOGI("HID", "SET_REPORT id=%d type=%d len=%d", report_id, report_type,
           bufsize);
  // Handle host -> device data here (e.g. LED feedback)
}

// Called after tud_hid_report() completes successfully
void tud_hid_report_complete_cb(uint8_t instance, const uint8_t *report,
                                uint16_t len) {
  ESP_LOGD("HID", "Report sent, len=%d", len);
}

void deck_hid_send_state(deck_input_report_t *report) {
  if (!tud_hid_ready())
    return;
  // Cast to raw bytes, skip the report ID (TinyUSB adds it)
  tud_hid_report(1, (uint8_t *)report, sizeof(deck_input_report_t));
}

void handle_received_command(uint8_t *buf, uint32_t len) {
  if (len == 0)
    return;

  uint8_t report_id = buf[0];

  switch (report_id) {
  case 0x10: // Example: LED control
    if (len < 2)
      return;
    uint8_t led_value = buf[1];
    ESP_LOGI("HID", "Set LED to %d", led_value);
    // TODO: call your LED driver here
    break;

  case 0x20: // Example: trigger some action
    if (len < 3)
      return;
    uint8_t param1 = buf[1];
    uint8_t param2 = buf[2];
    ESP_LOGI("HID", "Action triggered with params: %d, %d", param1, param2);
    // TODO: handle action
    break;

  default:
    printf("Unknown report ID: %d\n", report_id);
    break;
  }
}

void deck_cdc_read_task(void *params) {
  (void)params;
  uint8_t buf[64];

  for (;;) {
    if (tud_cdc_connected()) {
      ESP_LOGD("CDC", "Checking for data...");
      uint32_t count = tud_cdc_read(buf, sizeof(buf));
      if (count > 0) {
        handle_received_command(buf, count);
      }
    }
    ESP_LOGI("CDC", "No data, sleeping...");
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
