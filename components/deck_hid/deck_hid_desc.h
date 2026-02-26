#pragma once
#include "tusb.h"
#include <stdint.h>

// ===================== Device Descriptor =====================
static const tusb_desc_device_t device_desc = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,     // USB 2.0
    .bDeviceClass = 0x00, // Class defined per interface
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = 64, // EP0 max packet size
    .idVendor = 0x303A,
    .idProduct = 0x4001,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

// ===================== HID Report Descriptor =====================
static const uint8_t deck_hid_report_descriptor[] = {
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined)
    0x09, 0x01,       // Usage (Vendor Usage 1)
    0xA1, 0x01,       // Collection (Application)

    // Input Report ID 1: 8 buttons + 3 sliders
    0x85, 0x01,
    // 8 buttons (1 bit each)
    0x05, 0x09, 0x19, 0x01, 0x29, 0x08, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01,
    0x95, 0x08, 0x81, 0x02,
    // 3 sliders (8-bit each)
    0x05, 0x01, 0x09, 0x36, 0x09, 0x36, 0x09, 0x36, 0x15, 0x00, 0x25, 0x64,
    0x75, 0x08, 0x95, 0x03, 0x81, 0x02,

    // Feature Report ID 2 (buttons config)
    0x85, 0x02, 0x06, 0x00, 0xFF, 0x09, 0x10, 0x15, 0x00, 0x26, 0xFF, 0x00,
    0x75, 0x08, 0x95, 0x40, 0xB1, 0x02,

    // Feature Report ID 3 (sliders config)
    0x85, 0x03, 0x06, 0x00, 0xFF, 0x09, 0x11, 0x15, 0x00, 0x26, 0xFF, 0x00,
    0x75, 0x08, 0x95, 0x40, 0xB1, 0x02,

    0xC0};

#define DECK_HID_REPORT_DESC_LEN sizeof(deck_hid_report_descriptor)

// ===================== Composite Configuration Descriptor
// =====================
static const uint8_t composite_config_descriptor[] = {
    // Configuration Descriptor
    0x09, // bLength
    0x02, // bDescriptorType (CONFIGURATION)
    0x5C,
    0x00, // wTotalLength = 92 bytes
    0x03, // bNumInterfaces: HID + CDC(2)
    0x01, // bConfigurationValue
    0x00, // iConfiguration
    0x80, // bmAttributes (bus powered)
    0x32, // bMaxPower (100mA)

    // ================= HID Interface =================
    0x09, // bLength
    0x04, // bDescriptorType (INTERFACE)
    0x00, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x01, // bNumEndpoints
    0x03, // bInterfaceClass (HID)
    0x00, // bInterfaceSubClass
    0x00, // bInterfaceProtocol
    0x00, // iInterface

    // HID Descriptor
    0x09, // bLength
    0x21, // HID descriptor type
    0x11,
    0x01,                                       // bcdHID 1.11
    0x00,                                       // bCountryCode
    0x01,                                       // bNumDescriptors
    0x22,                                       // bDescriptorType (Report)
    (uint8_t)(DECK_HID_REPORT_DESC_LEN & 0xFF), // wDescriptorLength LSB
    (uint8_t)(DECK_HID_REPORT_DESC_LEN >> 8),   // wDescriptorLength MSB

    // HID Endpoint
    0x07, // bLength
    0x05, // bDescriptorType (ENDPOINT)
    0x81, // bEndpointAddress (IN 1)
    0x03, // bmAttributes (Interrupt)
    0x40,
    0x00, // wMaxPacketSize (64 bytes)
    0x0A, // bInterval (10 ms)

    // ================= CDC Control Interface =================
    0x09, // bLength
    0x04, // bDescriptorType (INTERFACE)
    0x01, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x01, // bNumEndpoints
    0x02, // bInterfaceClass (CDC)
    0x02, // bInterfaceSubClass (ACM)
    0x01, // bInterfaceProtocol (AT)
    0x00, // iInterface

    // CDC Header Functional Descriptor
    0x05,
    0x24,
    0x00,
    0x10,
    0x01,
    // CDC ACM Functional Descriptor
    0x04,
    0x24,
    0x02,
    0x02,
    // CDC Union Functional Descriptor
    0x05,
    0x24,
    0x06,
    0x01,
    0x02,
    // CDC Call Management Descriptor
    0x05,
    0x24,
    0x01,
    0x00,
    0x02,

    // CDC Notification Endpoint
    0x07,
    0x05,
    0x82,
    0x03,
    0x08,
    0x00,
    0x10,

    // ================= CDC Data Interface =================
    0x09,
    0x04,
    0x02,
    0x00,
    0x02,
    0x0A,
    0x00,
    0x00,
    0x00,

    // CDC Data OUT Endpoint
    0x07,
    0x05,
    0x03,
    0x02,
    0x40,
    0x00,
    0x00,
    // CDC Data IN Endpoint
    0x07,
    0x05,
    0x83,
    0x02,
    0x40,
    0x00,
    0x00,
};
