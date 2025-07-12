#include <EncButton.h>
#include <EEPROM.h>
#include "CustomHID.h"

// #define DEBUG

#define PIN_BT1 5
#define PIN_BT2 6
#define PIN_BT3 7
#define PIN_BT4 8
#define PIN_HALL 12
#define PIN_ENC_BT 4
#define PIN_ENC_A 2
#define PIN_ENC_B 3

#define PIN_LED_R 10
#define PIN_LED_G 11
#define PIN_LED_B 13

#define HOLD_FOR_LED_MS 3000
#define LED_FLASH_MS 100

CustomHID_ CustomHID;

VirtEncoder eb; // Encoder
Button btns[6]; // Buttons

int sh = 0; // Shuttle value

// RGB LED colors components
uint8_t red = 100;
uint8_t green = 100;
uint8_t blue = 100;

bool bt[6] = {false, false, false, false, false, false}; // Button states
bool dtr = false;                                        // Data to report

void isr()
{
  eb.tickISR(digitalRead(PIN_ENC_A), digitalRead(PIN_ENC_B));
}

void onOutReport(uint32_t value);

void changeLEDColor()
{
  analogWrite(PIN_LED_R, red);
  analogWrite(PIN_LED_G, green);
  analogWrite(PIN_LED_B, blue);
}

void saveLEDColor()
{
  EEPROM.write(1, red & 0xFF);
  EEPROM.write(2, green & 0xFF);
  EEPROM.write(3, blue & 0xFF);
}

void setup()
{
  CustomHID.setOutCallback(onOutReport);

  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), isr, CHANGE);
  eb.setEncType(EB_STEP1);
  eb.setEncISR(1);

  btns[0].init(PIN_HALL);
  btns[0].attach(btn_hall_cb);
  btns[1].init(PIN_ENC_BT);
  btns[1].setHoldTimeout(HOLD_FOR_LED_MS);
  btns[1].attach(btn_enc_cb);
  btns[2].init(PIN_BT1);
  btns[2].attach(btn1_cb);
  btns[3].init(PIN_BT2);
  btns[3].attach(btn2_cb);
  btns[4].init(PIN_BT3);
  btns[4].attach(btn3_cb);
  btns[5].init(PIN_BT4);
  btns[5].attach(btn4_cb);

#ifdef DEBUG
  Serial.begin(115200);
#endif

  uint8_t eeprom_inited = 0;
  eeprom_inited = EEPROM.read(0);
  if (eeprom_inited != 0x78)
  {
    EEPROM.write(0, 0x78);
    saveLEDColor();
  }

  red = EEPROM.read(1);
  green = EEPROM.read(2);
  blue = EEPROM.read(3);
  changeLEDColor();
}

void enc_cb()
{
  sh = (sh + eb.dir()) & 0xff;
  dtr = true;
#ifdef DEBUG
  Serial.print("Shuttle: ");
  Serial.println(sh);
#endif
}

void btn_enc_cb()
{
  btn_cb(1);
  // switch (btns[1].action())
  // {
  // case EB_HOLD:
  //   break;
  // case EB_RELEASE:
  //   break;
  // }
}

void btn1_cb()
{
  btn_cb(2);
}

void btn2_cb()
{
  btn_cb(3);
}

void btn3_cb()
{
  btn_cb(4);
}

void btn4_cb()
{
  btn_cb(5);
}

void btn_hall_cb()
{
  btn_cb(0);
}

void btn_cb(int bn)
{
  switch (btns[bn].action())
  {
  case EB_PRESS:
    if (!bt[bn])
    {
      bt[bn] = true;
      dtr = true;
#ifdef DEBUG
      Serial.println("Button " + String(bn) + " pressed.");
#endif
    }
    break;
  case EB_RELEASE:
    if (bt[bn])
    {
      bt[bn] = false;
      dtr = true;
#ifdef DEBUG
      Serial.println("Button " + String(bn) + " released.");
#endif
    }
    break;
  }
}

void onOutReport(uint32_t value)
{
  uint32_t command = value & 0xFF;
  uint32_t data1 = (value >> 8) & 0xFF;
  uint32_t data2 = (value >> 16) & 0xFF;
  uint32_t data3 = (value >> 24) & 0xFF;
  switch (command)
  {
  case 0x01: // Set RGB LED color
  case 0x02: // Set RGB LED color and save to EEPROM
    red = data1;
    green = data2;
    blue = data3;
    changeLEDColor();
    if (command == 0x02)
      saveLEDColor();
    break;
  default:
#ifdef DEBUG
    Serial.print("Unknown command: ");
    Serial.println(command);
#endif
    break;
  }
}

void loop()
{
  eb.tick();
  for (int i = 0; i < 6; i++)
    btns[i].tick();

  if (eb.turn())
    enc_cb();

  if (dtr)
  {
#ifdef DEBUG
    Serial.println("Data changed.");
#endif
    // buttonMasks: [0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200]
    CustomHID.sendReport(0x00, sh & 0xFF, (bt[5] ? 0x0200 : 0) | (bt[4] ? 0x0010 : 0) | (bt[3] ? 0x0020 : 0) | (bt[2] ? 0x0040 : 0) | (bt[1] ? 0x0080 : 0) | (bt[0] ? 0x0100 : 0));
    dtr = false;
  }
}
