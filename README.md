# HVL-BLE-ESS: Bluetooth Environmental Sensor

This project uses the M5Stack Atom S3 Lite and ENV Unit to create a Bluetooth Low Energy (BLE) Environmental Sensor. It broadcasts **temperature**, **humidity**, **pressure**, and **elevation** using the standard [Environmental Sensing Service (UUID: 0x181A)](https://www.bluetooth.com/specifications/specs/environmental-sensing-service-1-0/).

It is fully compliant with Bluetooth GATT specifications and encoded for compatibility with clients like **nRF Connect**, **Web Bluetooth**, or **custom dashboards**.

---

## 🔧 Hardware Requirements

- [M5Stack Atom S3 Lite](https://shop.m5stack.com/products/atom-s3-lite)
- [M5Stack ENV Unit (SHT40 + BMP280)](https://shop.m5stack.com/products/env-iv-unit-with-temperature-humidity-air-pressure-sensor-sht40-bmp280)

---

## 📡 Broadcasted GATT Services

| Service Name               | UUID     | Description                          |
|---------------------------|----------|--------------------------------------|
| Environmental Sensing     | `0x181A` | Core service for sensor readings     |

### Characteristics

| Characteristic   | UUID     | Format                 | Units       | Description              |
|------------------|----------|------------------------|-------------|--------------------------|
| Temperature       | `0x2A6E` | IEEE-11073 32-bit float | °C          | Ambient temperature      |
| Humidity          | `0x2A6F` | uint16 (0.01%)         | %RH         | Relative humidity        |
| Pressure          | `0x2A6D` | uint32                 | Pascal (Pa) | Atmospheric pressure     |
| Elevation         | `0x2A6C` | int16                  | meters (m)  | Estimated altitude       |

All values are **notified** and encoded according to the [Bluetooth SIG GATT specification](https://www.bluetooth.com/specifications/gatt/).

---

## 🚀 Getting Started

1. Flash the code to your M5 Atom S3 Lite using the Arduino IDE or PlatformIO.
2. Power the device with USB.
3. Open **nRF Connect** or your preferred BLE client.
4. Look for the device name: `HVL-BLE-ESS-XXXXXX`
5. Subscribe to the sensor characteristics to receive real-time updates.

---

## 🧪 Example Output
🌡 Temp: 22.94 °C  💧 Humidity: 54.3 %  🧭 Pressure: 1013.2 hPa  ⛰ Alt: 44.6 m

## ⚠️ License & Usage

This code is provided **for personal, educational, and research use only**.

> **🚫 Not licensed for commercial use or redistribution in commercial products.**

If you'd like to use this in a commercial application, please contact the author for permission.

---

## 👨‍🔬 Created by

**Hill Valley Labs**  
Built as a rapid prototype for BLE environmental sensing on ESP32 using M5Stack modules.

---

## 🧠 Resources

- [Bluetooth GATT Services](https://www.bluetooth.com/specifications/gatt/services/)
- [M5Stack Docs](https://docs.m5stack.com/)
- [Web Bluetooth API](https://web.dev/bluetooth/)