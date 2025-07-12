#include "CustomHID.h"

CustomHID_::CustomHID_()
  : PluggableUSBModule(2, 1, epType) {
  epType[0] = EP_TYPE_INTERRUPT_IN;
  epType[1] = EP_TYPE_INTERRUPT_OUT;
  PluggableUSB().plug(this);
  outCallback = nullptr;
}

int CustomHID_::getInterface(uint8_t* interfaceCount) {
  *interfaceCount += 1;
  // Интерфейсный дескриптор (9 байт)
  uint8_t interfaceDesc[] = {
    9, 4, pluggedInterface, 0x00, 1, 0x03, 0x00, 0x00, 0
  };
  // HID дескриптор (9 байт)
  uint8_t hidDesc[] = {
    9, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, (uint8_t)(sizeof(customHIDReportDescriptor)), (uint8_t)(sizeof(customHIDReportDescriptor) >> 8)
  };
  // Endpoint IN (7 байт)
  uint8_t epInDesc[] = {
    7, 5, (uint8_t)(0x80 | pluggedEndpoint), 0x03, USB_EP_SIZE, 0, 0x01
  };

  USB_SendControl(0, interfaceDesc, sizeof(interfaceDesc));
  USB_SendControl(0, hidDesc, sizeof(hidDesc));
  USB_SendControl(0, epInDesc, sizeof(epInDesc));
  return sizeof(interfaceDesc) + sizeof(hidDesc) + sizeof(epInDesc);
}

int CustomHID_::getDescriptor(USBSetup& setup) {
  if (setup.wValueH == HID_REPORT_DESCRIPTOR_TYPE) {
    return USB_SendControl(TRANSFER_PGM, customHIDReportDescriptor, sizeof(customHIDReportDescriptor));
  }
  return 0;
}

bool CustomHID_::setup(USBSetup& setup) {
  if (pluggedInterface != setup.wIndex) return false;
  if (setup.bmRequestType == REQUEST_DEVICETOHOST_STANDARD_INTERFACE && setup.bRequest == HID_GET_REPORT) {
    // Host requests current report
    return USB_SendControl(0, reportBuffer, sizeof(reportBuffer)) >= 0;
  }
  if (setup.bmRequestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE && setup.bRequest == HID_SET_REPORT) {
    uint8_t outBuf[4];
    if (USB_RecvControl(outBuf, 4) == 4 && outCallback) {
      uint32_t value =
        (uint32_t)outBuf[0] | ((uint32_t)outBuf[1] << 8) | ((uint32_t)outBuf[2] << 16) | ((uint32_t)outBuf[3] << 24);
      outCallback(value);
    }
    return true;
  }
  return false;
}

void CustomHID_::sendReport(uint8_t b1, uint8_t b2, uint16_t bools) {
  reportBuffer[0] = b1;
  reportBuffer[1] = b2;
  reportBuffer[2] = bools & 0xFF;
  reportBuffer[3] = (bools >> 8) & 0xFF;
  USB_Send(pluggedEndpoint | TRANSFER_RELEASE, reportBuffer, sizeof(reportBuffer));
}

void CustomHID_::setOutCallback(void (*cb)(uint32_t)) {
  outCallback = cb;
}
