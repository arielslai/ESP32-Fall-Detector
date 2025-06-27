#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

Adafruit_MPU6050 mpu;
unsigned long lastReadTime = 0; // start time
unsigned long spikeTime = 0; // time of the first spike
unsigned long fallDetectedTime = 0; 
unsigned long blinkStartTime = 0;
const unsigned long interval = 100; // time period between each acceleration reading
const float ACC_THRESHOLD = 10.0; // acceleration detected at impact lowerbound
const float STILL_ACC = 2.0; // acceleratoin at rest upperbound
const float STILL_GYRO = 0.5; // radial acceleration at rest upperbound
const float STILLNESS_DELAY = 1000; //1 second delay to monitor stillness
const float CANCEL_TIME = 5000; 
const int ledPin = 23; // output pin number
const int buttonPin = 18; // output button number
int blinkCount = 0;
bool spike = false;
bool fallDetected = false;
bool canceled = false;
bool blinkState = false; 

/* WiFi connection variables*/
const char* ssid = "INSERT_WIFI_SSID";
const char* password = "INSERT_WIFI_PASSWORD";
const char* apiKey = "INSERT_API_KEY"; // API Key from CallMeBot
const char* phoneNumber = "INSERT_PHONE_NUMBER"; //Phone number registered with CallMeBot



void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(buttonPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
}

void loop() {
  unsigned long now = millis();

  if (now - lastReadTime >= interval) { 
    lastReadTime = now;

    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    /* is there a spike in acceleration? */
    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = abs(a.acceleration.z);
    bool zFall = az > 20.0;
    float acc_Mag = abs(sqrt(ax*ax + ay*ay + az*az) - 9.81);

    /* Spike Detection */
    if (acc_Mag > ACC_THRESHOLD && !spike && !fallDetected && zFall) {
      spike = true;
      spikeTime = now;
      canceled = false;
      blinkCount = 0;  // Reset blink counter for new detection
    }

    /* Testing for stillness after spike is detected */
    if (spike && (now - spikeTime >= STILLNESS_DELAY)) {
      if (acc_Mag < STILL_ACC) {
        Serial.println("Fall detected! Press button to cancel SMS.");
        fallDetected = true;
        fallDetectedTime = now;
        blinkStartTime = now;
        blinkState = false;  // Start with LED off
        blinkCount = 0;       // Reset blink counter
        digitalWrite(ledPin, LOW);  // Ensure LED starts off
      }
      spike = false;
    }

    /* Handle fall detection state */
    if (fallDetected) {
      // Blink LED 5 times during cancellation window
      if (now - fallDetectedTime <= CANCEL_TIME) {
        if (blinkCount < 10) {  // 10 changes = 5 blinks (on+off)
          if (now - blinkStartTime >= 500) {  // Change state every 500ms
            blinkStartTime = now;
            blinkState = !blinkState;
            digitalWrite(ledPin, blinkState ? HIGH : LOW);
            blinkCount++;
          }
        }
      }

      // Check for button press
      if (digitalRead(buttonPin) == LOW && !canceled) {
        Serial.println("Fall canceled.");
        digitalWrite(ledPin, LOW);
        fallDetected = false;
        canceled = true;
        blinkCount = 0;  // Reset blink counter
      }
      // Check if cancel window has expired
      else if ((now - fallDetectedTime > CANCEL_TIME) && !canceled) {
        Serial.println("Fall confirmed. Sending SMS...");
        digitalWrite(ledPin, LOW);
        sendSMS();
        fallDetected = false;
        blinkCount = 0;  // Reset blink counter
      }
    }
  }
}

void sendSMS() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Construct the URL for your SMS API
    String url = "http://sms.example.com/api/send"; // Replace with your SMS provider's API endpoint
    String message = "ALERT: Fall detected! Immediate attention may be needed.";
    
    // Add parameters (format depends on your SMS provider)
    String postData = "apiKey=" + String(apiKey);
    postData += "&to=" + String(phoneNumber);
    postData += "&message=" + message;
    
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    int httpResponseCode = http.POST(postData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("SMS sent successfully");
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error sending SMS");
      Serial.println("Error code: " + String(httpResponseCode));
    }
    
    http.end();
  } else {
    Serial.println("WiFi not connected - cannot send SMS");
  }
}
