#include <WiFi.h>
#include <HTTPClient.h>

// ======== Configuration ======== //
const char* ssid = "SSID";               // Your working SSID
const char* password = "PASSWORD";       // WiFi password
const char* phoneNumber = "PHONENUMBER";    // No '+' prefix
const char* apiKey = "APIKEY";             // CallMeBot API key
const int ledPin = 2;                       // Built-in LED

// ======== Debug Tracing ======== //
void trace(String message) {
  Serial.print("[TRACE] ");
  Serial.println(message);
}

void blinkStatus(int count, int duration) {
  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW);
    delay(duration);
  }
}

// ======== WiFi Connection ======== //
bool connectWiFi() {
  trace("Initializing WiFi...");
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  
  trace("Scanning networks...");
  int n = WiFi.scanNetworks();
  trace("Found " + String(n) + " networks");
  for (int i = 0; i < n; i++) {
    trace(String(i+1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + "dBm)");
  }

  trace("Connecting to: " + String(ssid));
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    trace("Connected! IP: " + WiFi.localIP().toString());
    trace("Signal strength: " + String(WiFi.RSSI()) + "dBm");
    return true;
  } else {
    trace("Connection failed! Status: " + String(WiFi.status()));
    return false;
  }
}

// ======== WhatsApp Messenger ======== //
bool sendWhatsApp(String message) {
  trace("Preparing WhatsApp message...");
  
  // URL encode message (spaces to +)
  message.replace(" ", "+");
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + String(phoneNumber) + 
               "&text=" + message + "&apikey=" + String(apiKey);

  trace("Request URL: " + url);
  
  HTTPClient http;
  http.begin(url);
  trace("HTTP connection started");

  trace("Sending GET request...");
  int httpCode = http.GET();
  
  String response = http.getString();
  trace("Response code: " + String(httpCode));
  trace("Response body: " + response);
  
  http.end();
  trace("HTTP connection closed");

  if (httpCode == 200) {
    trace("Message sent successfully!");
    return true;
  } else {
    trace("Failed to send message");
    return false;
  }
}

// ======== Main Program ======== //
void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // Wait for serial port to connect
  delay(1000);
  
  pinMode(ledPin, OUTPUT);
  trace("System initialized");

  if (connectWiFi()) {
    if (sendWhatsApp("ESP32 test message with tracing!")) {
      blinkStatus(3, 200);  // Success pattern
    } else {
      blinkStatus(10, 100); // Error pattern
    }
  } else {
    blinkStatus(5, 500);    // Connection failed pattern
  }
}

void loop() {
  // Optional: Resend message every 60 seconds
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 60000) {
    lastSend = millis();
    if (WiFi.status() == WL_CONNECTED) {
      sendWhatsApp("Periodic message from ESP32");
    } else {
      trace("WiFi disconnected, attempting reconnect...");
      connectWiFi();
    }
  }
  delay(100);
}