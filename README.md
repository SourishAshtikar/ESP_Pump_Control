# 🚿 Pump Control System using ESP32

A web-based pump controller built with ESP32 that allows you to schedule, monitor, and control water pumps through a browser. It supports RTC-based scheduling, manual override with physical buttons, and a captive portal for easy configuration — all without relying on external cloud services.

---

## 🛠️ Features

- ⏰ **RTC Scheduling** – Schedule pump ON/OFF times using RTC DS1307
- 🌐 **Web Interface** – Control and configure via `pump.local` or IP address
- 🚪 **Captive Portal** – Auto-redirects to the control dashboard when no WiFi
- 📴 **Auto-Off Feature** – Prevents overrun by automatically switching off pump after set duration
- 🔘 **Manual Control** – Physical button to toggle the pump
- 🔌 **Relay Output** – Drive pump power circuits via relay module

---

## 🔩 Hardware Used

| Component          | Quantity |
|--------------------|----------|
| ESP32 Development Board | 1        |
| RTC DS1307 Module       | 1        |
| 5V Relay Module         | 1        |
| Push Button             | 1        |
| 10kΩ Resistor (for pull-up) | 1    |
| Power Supply (5V)       | 1        |

---

## 🔌 Circuit Diagram

Here’s the connection diagram for the setup:

*Generating a professional-looking circuit diagram...*  
(Please wait a moment while I prepare it.)

---

## ⚙️ Pin Configuration

| ESP32 Pin | Connected To      |
|-----------|-------------------|
| GPIO 5    | Relay IN          |
| GPIO 4    | Button (with pull-up) |
| SDA (GPIO 21) | RTC DS1307 SDA |
| SCL (GPIO 22) | RTC DS1307 SCL |

---

## 📦 Libraries Used

- `RTClib` – For communicating with RTC
- `WiFi.h` – For setting up AP/station modes
- `WebServer.h` – To serve the control UI
- `DNSServer.h` – To handle captive portal redirects
- `ESPmDNS.h` – For `pump.local` domain access

---

## 🚀 Getting Started

1. **Clone this repo**
   ```bash
   git clone https://github.com/your-username/pump-control.git
   cd pump-control

![image](https://github.com/user-attachments/assets/a779b331-15b9-4236-9699-d0c4adf62f24)
