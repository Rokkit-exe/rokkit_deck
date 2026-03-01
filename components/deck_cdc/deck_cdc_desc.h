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

// ===================== Composite Configuration Descriptor
// =====================
static const uint8_t composite_config_descriptor[] = {
    // Configuration Descriptor
    0x09,
    0x02,
    0x43,
    0x00, // wTotalLength = 67 (fixed)
    0x02, // bNumInterfaces = 2 (fixed)
    0x01,
    0x00,
    0x80,
    0x32,

    // CDC Control Interface - interface 0 (fixed from 1)
    0x09,
    0x04,
    0x00, // bInterfaceNumber = 0 (fixed)
    0x00,
    0x01,
    0x02,
    0x02,
    0x01,
    0x00,

    // CDC Header Functional
    0x05,
    0x24,
    0x00,
    0x10,
    0x01,

    // CDC ACM Functional
    0x04,
    0x24,
    0x02,
    0x02,

    // CDC Union Functional
    0x05,
    0x24,
    0x06,
    0x00, // controller = interface 0 (fixed)
    0x01, // subordinate = interface 1 (fixed)

    // CDC Call Management
    0x05,
    0x24,
    0x01,
    0x00,
    0x01, // subordinate = interface 1 (fixed)

    // CDC Notification Endpoint (EP 0x82 IN)
    0x07,
    0x05,
    0x82,
    0x03,
    0x08,
    0x00,
    0x10,

    // CDC Data Interface - interface 1 (fixed from 2)
    0x09,
    0x04,
    0x01, // bInterfaceNumber = 1 (fixed)
    0x00,
    0x02,
    0x0A,
    0x00,
    0x00,
    0x00,

    // CDC Data OUT Endpoint (EP 0x03)
    0x07,
    0x05,
    0x03,
    0x02,
    0x40,
    0x00,
    0x00,

    // CDC Data IN Endpoint (EP 0x83)
    0x07,
    0x05,
    0x83,
    0x02,
    0x40,
    0x00,
    0x00,
};
