# Custom HID Controller for Arduino Micro (ATmega32u4)

This project implements a **custom USB HID controller** using an **Arduino Micro** (ATmega32u4) with no external HID libraries required. The controller features:

- 🔘 Rotary Encoder  
- 🖲️ 5 Push Buttons  
- 🧲 Hall Sensor (for zero-position detection)  
- 🌈 Controlled RGB Lighting
- 🔌 Bi-directional HID communication (IN/OUT reports)

---

## 🔧 Features

### Custom HID Protocol

- **IN Report** (Device → Host):  
  - 2 bytes of general-purpose data  
  - 2 bytes of button states (16 booleans)  

- **OUT Report** (Host → Device):  
  - 4 bytes of control data (e.g., RGB values, mode flags, etc.)

### HID Descriptor

- Fully custom HID report descriptor using **Vendor Defined Page (0xFF00)**
- No use of `HID-Project` or `HID-API` libraries — low-level USB control via `PluggableUSBModule`

---

## 🧩 Hardware

- **Microcontroller**: Arduino Micro / Leonardo (ATmega32u4)  
- **Inputs**:
  - Rotary encoder (A/B)
  - 5 digital push buttons
  - Hall sensor (analog or digital input for zero detection)
- **Outputs**:
  - RGB LEDs

---

## 📦 USB Reports

### IN Report (to Host)
- `Byte 0`: General-purpose data (e.g., encoder delta)
- `Byte 1`: Additional status or reserved
- `Byte 2-3`: Button states (each bit = 1 button, 16 total)

### OUT Report (from Host)
- `Byte 0-3`: Host-controlled data, such as:
  - LED brightness / color
  - Mode/state commands
  - Vibration or feedback flags

---

## 🔌 Installation

1. Open the project in the **Arduino IDE**
2. Select **Arduino Micro** as the board
3. Upload the sketch
4. The device will appear as a **HID-compliant device** (not a serial port)

---

## 💡 Usage Example

- Host software (e.g., game, simulator, or custom control panel) sends an OUT report to set LED color
- Controller sends IN reports with button states and encoder values
- Hall sensor detects when the encoder is in its "zero" or home position

---

## 📁 Files

- `HIDControllerV2.ino` — Main Arduino sketch with `setup()` and `loop()`
- `CustomHID` class — Implements `PluggableUSBModule` with custom IN/OUT report handling
- HID descriptor embedded as a raw byte array

---

## 💻 Host Communication

You can use any HID-capable host software or scripting language. For example, in Python:

```python
import hid

dev = hid.device()
dev.open(vendor_id, product_id)
dev.write([0x00, r, g, b])  # Send RGB control
data = dev.read(4)          # Read encoder + buttons
