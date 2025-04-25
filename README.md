# MIE438_Temp_Home_Device

# Smart Home Automation System

This project is a smart home automation prototype built around the ESP32 microcontroller. It features real-time control of RGB LED lighting, temperature sensing, fan and heater actuation in both manual and automatic modes, and a dynamic web-based interface for interaction.

---

## 🧠 System Overview

The system is divided into five primary modules:

1. **Web Page** – Serves an HTML-based dashboard interface.
2. **WiFi Control** – Connects ESP32 to a local network and runs a web server.
3. **LED Control** – Toggles a NeoPixel RGB LED to simulate smart lighting.
4. **Temperature and Fan Control (Manual Mode)** – Lets users manually control a heater and fan.
5. **Temperature and Fan Control (Automatic Mode)** – Automatically adjusts fan/heater based on temperature thresholds using hysteresis.

---

## 📚 Code Libraries Used

| Library                | Description                                                                 | Module Used In                          |
|------------------------|-----------------------------------------------------------------------------|------------------------------------------|
| `Adafruit_NeoPixel.h`  | Controls NeoPixel RGB LEDs                                                  | LED Control                              |
| `Wire.h`               | Enables I2C communication with peripherals                                  | Manual & Automatic Temperature Control   |
| `Adafruit_TMP117.h`    | Interfaces with TMP117 temperature sensor                                   | Manual & Automatic Temperature Control   |
| `WiFi.h`               | Manages ESP32 WiFi and web server operations                                | Web Page & WiFi Control                  |

---

## 🌐 Web Dashboard Interface

- Displays a control panel upon WiFi connection
- Users can toggle between Manual and Automatic modes
- Real-time temperature is displayed and updated every 2 seconds
- Button color changes indicate the on/off status of each device

---

## ⚙️ Manual Mode

- Direct control over:
  - RGB LED (on/off)
  - Fan (off or full speed, PWM = 255)
  - Heater (on/off)
- All peripherals can be toggled independently

---

## 🤖 Automatic Mode

- Uses preset thresholds with hysteresis:
  - If temp > 26°C → Fan runs at full speed
  - If temp < 23°C → Heater activates, Fan runs at low speed (PWM = 1)
- Intended to simulate intelligent environmental adjustment

---

## 🔈 Planned Feature (Not Implemented)

**Audio Feedback Module**
- Was intended to play sounds upon user interaction
- Could not be deployed due to a conflict between WiFi and speaker modules
- Sample code is located in `future_implementation/`

---

## 🧑‍💻 User Interaction Summary

- Connect ESP32 to your WiFi network (update SSID & password in code)
- Access the device’s IP to open the control dashboard
- Switch modes and monitor changes in real-time
- Use Manual Mode for direct control or Automatic Mode for temperature-triggered automation

---

## 🎬 Demo

A demo video of the working system:
https://www.youtube.com/watch?v=AboBtPy3Abc

---

## 📌 Notes

- Ensure both the ESP32 and your accessing device are on the same local network
- If adding audio, resolve the WiFi-speaker GPIO conflict in future iterations

---

