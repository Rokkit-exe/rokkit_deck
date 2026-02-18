#pragma once
#include "tusb.h"
#include <stdint.h>

// Custom HID descriptor for Stream Deck style device
static const uint8_t deck_hid_report_descriptor[] = {

    // ================= TOP LEVEL =================
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,       // Usage (Vendor Usage 1)
    0xA1, 0x01,       // Collection (Application)

    // =====================================================
    // INPUT REPORT (ID 1) - Buttons + Sliders
    // =====================================================
    0x85, 0x01, //   Report ID (1)

    // -----------------------------
    // 8 Buttons (1 bit each)
    // -----------------------------
    0x05, 0x09, //   Usage Page (Button)
    0x19, 0x01, //   Usage Minimum (Button 1)
    0x29, 0x08, //   Usage Maximum (Button 8)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1 bit)
    0x95, 0x08, //   Report Count (8 buttons)
    0x81, 0x02, //   Input (Data, Variable, Absolute)

    // -----------------------------
    // 3 Sliders (8-bit each)
    // -----------------------------
    0x05, 0x01, //   Usage Page (Generic Desktop)
    0x09, 0x36, //   Usage (Slider)
    0x09, 0x36, //   Usage (Slider)
    0x09, 0x36, //   Usage (Slider)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x64, //   Logical Maximum (100)
    0x75, 0x08, //   Report Size (8 bits)
    0x95, 0x03, //   Report Count (3)
    0x81, 0x02, //   Input (Data, Variable, Absolute)

    // =====================================================
    // FEATURE REPORT (ID 2) - Button Configuration
    // =====================================================
    0x85, 0x02, //   Report ID (2)

    0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined)
    0x09, 0x10,       //   Usage (Button Config)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8 bits)
    0x95, 0x40,       //   Report Count (64 bytes)
    0xB1, 0x02,       //   Feature (Data, Variable, Absolute)

    // =====================================================
    // FEATURE REPORT (ID 3) - Slider Configuration
    // =====================================================
    0x85, 0x03, //   Report ID (3)

    0x06, 0x00, 0xFF, //   Usage Page (Vendor Defined)
    0x09, 0x11,       //   Usage (Slider Config)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8 bits)
    0x95, 0x40,       //   Report Count (64 bytes)
    0xB1, 0x02,       //   Feature (Data, Variable, Absolute)

    0xC0 // End Collection
};

#define DECK_HID_REPORT_DESC_LEN sizeof(deck_hid_report_descriptor)

// The full USB config descriptor
static const uint8_t deck_hid_config_descriptor[] = {
    // Config descriptor (9 bytes)
    9,
    TUSB_DESC_CONFIGURATION,
    U16_TO_U8S_LE(9 + 9 + 9 + 7), // total length
    1,                            // num interfaces
    1,                            // config number
    0,                            // string index
    0x80,                         // attributes (bus powered)
    50,                           // max power (100mA)

    // Interface descriptor (9 bytes)
    9,
    TUSB_DESC_INTERFACE,
    0,                     // interface number
    0,                     // alternate setting
    1,                     // num endpoints
    TUSB_CLASS_HID,        // class
    0,                     // subclass (0 = no boot)
    HID_ITF_PROTOCOL_NONE, // protocol
    0,                     // string index

    // HID descriptor (9 bytes)
    9,
    HID_DESC_TYPE_HID,
    U16_TO_U8S_LE(0x0111),                   // HID version 1.11
    0,                                       // country code
    1,                                       // num descriptors
    HID_DESC_TYPE_REPORT,                    // descriptor type
    U16_TO_U8S_LE(DECK_HID_REPORT_DESC_LEN), // descriptor length

    // Endpoint descriptor (7 bytes)
    7,
    TUSB_DESC_ENDPOINT,
    0x81,                // EP 1 IN
    TUSB_XFER_INTERRUPT, // interrupt transfer
    U16_TO_U8S_LE(64),   // max packet size
    10,                  // polling interval (10ms)
};

// Report ID 1:
// buttons (1 byte)
// slider1 (1 byte)
// slider2 (1 byte)
// slider3 (1 byte)
