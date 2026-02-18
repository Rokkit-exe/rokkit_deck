#include "deck_hid.h"
#include "common/tusb_types.h"
#include "deck_hid_desc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_private/usb_phy.h"
#include "hal/usb_phy_types.h"
#include "hal/usb_serial_jtag_ll.h"
#include "soc/usb_serial_jtag_reg.h"
#include "tinyusb_default_config.h"
#include "tusb.h"

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

static const tusb_desc_device_t device_desc = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x303A,
    .idProduct = 0x4001,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const char lang_descriptor[] = {0x09, 0x04};
static const char *descriptor_strings[] = {
    [0] = lang_descriptor, // static, not a compound literal
    [1] = "Freenove ESP32-S3", [2] = "ESP32 Stream Deck",
    [3] = "1234567890AB",      [4] = NULL,
};

void deck_hid_init(void) {
  // Claim PHY for OTG before JTAG does
  usb_phy_config_t phy_config = {
      .controller = USB_PHY_CTRL_OTG,
      .target = USB_PHY_TARGET_INT,
      .otg_mode = USB_OTG_MODE_DEVICE,
      .otg_speed = USB_PHY_SPEED_FULL,
  };
  usb_phy_handle_t phy_handle;
  ESP_ERROR_CHECK(usb_new_phy(&phy_config, &phy_handle));

  ESP_LOGI("HID", "Starting TinyUSB init...");
  tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG(device_event_handler);

  tusb_cfg.port = 0; // Use USB OTG port 0
  tusb_cfg.descriptor.device = &device_desc;
  tusb_cfg.descriptor.string = descriptor_strings;
  tusb_cfg.descriptor.string_count = 4;
  tusb_cfg.descriptor.full_speed_config = deck_hid_config_descriptor;
  tusb_cfg.phy.skip_setup = true; // Skip USB PHY setup since we do it manually

  esp_err_t err = tinyusb_driver_install(&tusb_cfg);
  if (err != ESP_OK) {
    ESP_LOGE("HID", "Failed to initialize TinyUSB driver: %s",
             esp_err_to_name(err));
    return;
  }
  ESP_LOGI("HID", "HID device initialized");
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
