# 🛡️ Fall Detection System with ESP32 and MPU6050

This project implements a wearable fall detection system using an ESP32 microcontroller and an MPU6050 accelerometer/gyroscope module. It detects potential falls and sends SMS alerts using a WiFi connection if the event is not canceled within a few seconds.

## 📌 Features

- 📉 Detects spikes in acceleration and stillness to determine falls
- 💡 Blinks LED during a cancellation window (5 seconds)
- 🔘 Allows cancellation via physical button
- 📲 Sends SMS alerts using CallMeBot API over WiFi
- 🔧 Customizable thresholds and timing parameters

---

## 🔧 Hardware Required

- ESP32 microcontroller
- MPU6050 accelerometer/gyroscope sensor
- Push button
- LED
- Resistors (as needed)
- Breadboard and jumper wires
- Micro USB cable for ESP32

---

## 📈 How It Works

1. **Impact Detection**: The device monitors for a spike in acceleration using a threshold (`ACC_THRESHOLD`).
2. **Stillness Check**: After the spike, the system checks for stillness for a short delay (`STILLNESS_DELAY`).
3. **User Cancellation**: If still, it triggers a cancel window (5 seconds) during which the LED blinks and the user can press a button to cancel.
4. **Alert Sending**: If no cancellation occurs, it sends an SMS alert using an HTTP POST request to the CallMeBot API.

---

## 📲 Setting Up CallMeBot

1. Save the phone number with CallMeBot's number.
2. Start a WhatsApp chat with the bot and send:
   `I allow callmebot to send me messages`
3. You’ll receive an API key in response.
4. Update the following placeholders in your code:
   ```cpp
   const char* ssid = "INSERT_WIFI_SSID";
   const char* password = "INSERT_WIFI_PASSWORD";
   const char* apiKey = "INSERT_API_KEY";
   const char* phoneNumber = "INSERT_PHONE_NUMBER";


   ## 🛠️ Setup Instructions

   - Clone this repo or copy the code to your Arduino IDE.
   - Connect your MPU6050 to the ESP32 via I2C:  
     - `SDA` to `GPIO 21`  
     - `SCL` to `GPIO 22` (default I2C pins)
   - Connect an LED to `GPIO 23` and a button to `GPIO 18` with a pull-up resistor.
   - Install the following libraries in the Arduino IDE:
     - `Adafruit MPU6050`
     - `Adafruit Sensor`
     - `WiFi`
     - `HTTPClient`
   - Upload the code to your ESP32.
   - Open the Serial Monitor at `115200` baud to observe real-time logs and sensor data.

   ---

   ## : 🔌Connection Diagram
   
   | Component   | ESP32 Pin             | Notes                                    |
   | ----------- | --------------------- | ---------------------------------------- |
   | **MPU6050** | `SDA` → GPIO 21       | I2C data line                            |
   |             | `SCL` → GPIO 22       | I2C clock line                           |
   |             | `VCC` → 3.3V          | Use 3.3V from ESP32 (not 5V)             |
   |             | `GND` → GND           | Ground                                   |
   | **LED**     | `Anode (+)` → GPIO 23 | Use a 220Ω resistor in series with anode |
   |             | `Cathode (-)` → GND   | Connect to ground                        |
   | **Button**  | One leg → GPIO 18     | Configure with `INPUT_PULLUP`            |
   |             | Other leg → GND       | Pulls pin low when pressed               |


   ## 📏 Threshold & Timing Parameters (tweakable in code)

   | Parameter         | Description                                | Default Value |
   |------------------|--------------------------------------------|---------------|
   | `ACC_THRESHOLD`  | Minimum acceleration to detect spike       | `10.0`        |
   | `STILL_ACC`      | Max acceleration considered as still       | `2.0`         |
   | `STILL_GYRO`     | Max gyro value considered as still         | `0.5`         |
   | `STILLNESS_DELAY`| Delay to check for stillness (ms)          | `1000`        |
   | `CANCEL_TIME`    | Time window for cancel (ms)                | `5000`        |

   ---

   ## 🧠 Future Improvements

   - Fall classification by direction/type
   - Battery-powered wearable with power optimization
   - Integration with cloud services for logging
   - Mobile app notification instead of SMS


   ---

   ## 🙋‍♀️ Questions?

   Feel free to open an issue or contact me if you need help setting it up or modifying the detection logic.
