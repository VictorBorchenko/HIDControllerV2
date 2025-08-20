#include <EncButton.h>
#include <EEPROM.h>
#include "CustomHID.h"

// #define DEBUG
#define VERSION 1
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

#define PIN_LED2 9 // Status LED

#define LED_FLASH_MS 200

// #define HOLD_FOR_LED_MS 3000

CustomHID_ CustomHID;

VirtEncoder eb; // Encoder
Button btns[6]; // Buttons

int sh = 0; // Shuttle value

uint8_t red = 100; // RGB LED colors components
uint8_t green = 100;
uint8_t blue = 100;
uint8_t brightness = 160;
volatile float k_red = 1.0;
volatile float k_green = 1.0;
volatile float k_blue = 1.0;

unsigned long led_flash_t = 0;
bool led_flash = false;
bool led_on = true;

bool bt[6] = {false, false, false, false, false, false}; // Button states
bool dtr = false;                                        // Data to report

void isr()
{
  eb.tickISR(digitalRead(PIN_ENC_A), digitalRead(PIN_ENC_B));
}

void saveWB()
{
  EEPROM.put(5, k_red);
  EEPROM.put(9, k_green);
  EEPROM.put(13, k_blue);
  updateFeatureReport();
}

void loadWB()
{
  EEPROM.get(5, k_red);
  EEPROM.get(9, k_green);
  EEPROM.get(13, k_blue);
}

void changeLEDColor(uint8_t r = red, uint8_t g = green, uint8_t b = blue)
{
  analogWrite(PIN_LED_R, k_red * r);
  analogWrite(PIN_LED_G, k_green * g);
  analogWrite(PIN_LED_B, k_blue * b);
}

void changeWB(uint8_t k_r, uint8_t k_g, uint8_t k_b)
{
  // WB correction: коэффициенты 0..255, преобразуем в 0.1 - 1.0
  k_red = 0.1 + 0.9 * k_r / 255.0;
  k_green = 0.1 + 0.9 * k_g / 255.0;
  k_blue = 0.1 + 0.9 * k_b / 255.0;
  changeLEDColor(red, green, blue);
  saveWB();
}

void changeLed2(uint8_t b = brightness)
{
  analogWrite(PIN_LED2, b);
}

void saveLEDColor(uint8_t r, uint8_t g, uint8_t b)
{
  red = r;
  green = g;
  blue = b;
  EEPROM.write(1, r);
  EEPROM.write(2, g);
  EEPROM.write(3, b);
  updateFeatureReport();
}

void saveBrightness(uint8_t b)
{
  brightness = b;
  EEPROM.write(4, b);
  updateFeatureReport();
}

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), isr, CHANGE);
  eb.setEncType(EB_STEP1);
  eb.setEncISR(1);

  btns[0].init(PIN_HALL);
  btns[0].attach(btn_hall_cb);
  btns[1].init(PIN_ENC_BT);
  // btns[1].setHoldTimeout(HOLD_FOR_LED_MS);
  btns[1].attach(btn_enc_cb);
  btns[2].init(PIN_BT1);
  btns[2].attach(btn1_cb);
  btns[3].init(PIN_BT2);
  btns[3].attach(btn2_cb);
  btns[4].init(PIN_BT3);
  btns[4].attach(btn3_cb);
  btns[5].init(PIN_BT4);
  btns[5].attach(btn4_cb);

  uint8_t eeprom_inited = 0;
  eeprom_inited = EEPROM.read(0);
  if (eeprom_inited != 0x79)
  {
    EEPROM.write(0, 0x79);
    saveLEDColor(red, green, blue);
    saveBrightness(brightness);
    saveWB();
  }
  else
  {
    red = EEPROM.read(1);
    green = EEPROM.read(2);
    blue = EEPROM.read(3);
    brightness = EEPROM.read(4);
    loadWB();
  }
  changeLEDColor(red, green, blue);
  changeLed2(0);

  CustomHID.setOutCallback(onOutReport);
  CustomHID.setFeatureCallback(onFeatureReport);
  updateFeatureReport();
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
    changeLEDColor(data1, data2, data3);
    break;
  case 0x02: // Set RGB LED color and save to EEPROM
    saveLEDColor(data1, data2, data3);
    changeLEDColor();
    break;
  case 0x03: // Set RGB LED to saved color
    changeLEDColor();
    break;
  case 0x04: // Flash mode
    led_flash = data1 != 0;
    if (!led_flash)
      changeLEDColor();
  case 0x05: // Set LED2 brightness
    changeLed2(data1);
    break;
  case 0x06: // Set LED2 brightness and save to EEPROM
    saveBrightness(data1);
    changeLed2();
    break;
  case 0x07: // Set LED2 to saved brightness
    changeLed2();
    break;
  case 0x08: // WB correction
    changeWB(data1, data2, data3);
    break;
  default:
#ifdef DEBUG
    Serial.print("Unknown command: ");
    Serial.println(command);
#endif
    break;
  }
}

void onFeatureReport(const uint8_t *data, size_t len)
{
#ifdef DEBUG
  Serial.print("Feature Report received: ");
  for (size_t i = 0; i < len; i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
}

void updateFeatureReport()
{
  uint8_t featureReport[16];
  featureReport[0] = VERSION;
  featureReport[1] = red;
  featureReport[2] = green;
  featureReport[3] = blue;
  featureReport[4] = brightness;
  // WB coefficients scaled to 0..255
  featureReport[5] = (uint8_t)round((255.0 * (k_red - 0.1) / 0.9));
  featureReport[6] = (uint8_t)round((255.0 * (k_green - 0.1) / 0.9));
  featureReport[7] = (uint8_t)round((255.0 * (k_blue - 0.1) / 0.9));
  for (int i = 8; i < 16; i++)
    featureReport[i] = 0; // Fill the rest with zeros
  CustomHID.setFeatureReport(featureReport, sizeof(featureReport));
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
    // buttonMasks: [0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x8000]
    CustomHID.sendReport(0x00, sh & 0xFF, (bt[5] ? 0x0001 : 0) | (bt[4] ? 0x0010 : 0) | (bt[3] ? 0x0020 : 0) | (bt[2] ? 0x0040 : 0) | (bt[1] ? 0x0080 : 0) | (bt[0] ? 0x0100 : 0));

    dtr = false;
  }
  if (led_flash)
  {
    if (led_flash_t < millis())
    {
      led_flash_t = millis() + LED_FLASH_MS;
      led_on = !led_on;
      changeLEDColor(led_on ? red : 0, led_on ? green : 0, led_on ? blue : 0);
    }
  }
}
