#include "CustomHID.h"

CustomHID_ CustomHID;


volatile uint32_t lastReceivedOut = 0;
void onOutReport(uint32_t value) {
  lastReceivedOut = value;
}

void setup() {
  CustomHID.setOutCallback(onOutReport);
  lastReceivedOut = 0x11223344;
}

void loop() {
  // Пример отправки IN-отчёта (2 байта + 16 boolean)
  uint8_t byte1 = random(255); //lastReceivedOut & 0xFF;
  uint8_t byte2 = lastReceivedOut >> 8 & 0xFF;
  uint16_t bools = lastReceivedOut >> 16 & 0xFFFF;
  CustomHID.sendReport(byte1, byte2, bools);
  delay(100);
}