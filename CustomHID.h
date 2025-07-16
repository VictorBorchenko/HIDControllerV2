// CustomHID.h
#pragma once

#include <Arduino.h>
#include "PluggableUSB.h"

// HID Report Descriptor
const uint8_t customHIDReportDescriptor[] PROGMEM = {
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined)
    0x09, 0x01,       // Usage (Vendor Usage 1)
    0xA1, 0x01,       // Collection (Application)
    // OUT report (host -> device)
    0x09, 0x02,       //   Usage (Vendor Usage 2)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x04,       //   Report Count (4 bytes)
    0x91, 0x02,       //   Output (Data,Var,Abs)
    // IN report (device -> host)
    0x09, 0x03,       //   Usage (Vendor Usage 3)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xFF,       //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x02,       //   Report Count (2) - 2 bytes
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x09, 0x04,       //   Usage (Vendor Usage 4)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x10,       //   Report Count (16) - 16 bits
    0x81, 0x02,       //   Input (Data,Var,Abs)
                      // Feature report (16 байт)
    0x09, 0x05,       //   Usage (Vendor Usage 5)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8 бит)
    0x95, 0x10,       //   Report Count (16 байт)
    0xB1, 0x02,       //   Feature (Data,Var,Abs)
    0xC0              // End Collection

};

#define USB_DEVICE_CLASS_HUMAN_INTERFACE 0x03
#define HID_SUBCLASS_NONE 0x00
#define HID_PROTOCOL_NONE 0x00
#define HID_GET_REPORT 0x01
#define HID_SET_REPORT 0x09
#define HID_REPORT_DESCRIPTOR_TYPE 0x22

#ifndef EPTYPE1
#define EPTYPE1 7
#define EPTYPE0 6
#define EPDIR 0
#endif

// Custom HID class
class CustomHID_ : public PluggableUSBModule
{
public:
  CustomHID_(void);
  int getInterface(uint8_t *interfaceCount) override;
  int getDescriptor(USBSetup &setup) override;
  bool setup(USBSetup &setup) override;
  void sendReport(uint8_t b1, uint8_t b2, uint16_t bools);
  void setOutCallback(void (*cb)(uint32_t));
  void setFeatureReport(const uint8_t *data, size_t len);
  void getFeatureReport(uint8_t *data, size_t len);

private:
  uint8_t epType[2];
  uint8_t reportBuffer[4];
  uint8_t featureReportBuffer[16]; // для хранения Feature Report
  void (*outCallback)(uint32_t);
};
