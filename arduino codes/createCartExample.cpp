#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SSD1306.h>
#include <qrcodeoled.h>

#define BUTTON_PIN 4

// WiFi
const char* ssid = "SS2";
const char* password = "SamSwap1603";

const String BASE_URL = "https://api-ktoxqz34xq-el.a.run.app";

String cartId = "";

// OLED (ThingPulse library)
SSD1306 display(0x3c, 21, 22);

// QR object
QRcodeOled qr(&display);

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  connectWiFi();

  // OLED init
  display.init();
  display.clear();
  display.display();

  // QR init
  qr.init();

  Serial.println("Ready. Press button...");
}

// ---------------- LOOP ----------------

void loop() {
  handleButton();
}

// ---------------- BUTTON ----------------

void handleButton() {
  static bool lastState = HIGH;

  bool currentState = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW) {
    delay(1000);

    Serial.println("Creating cart...");
    createCart();
  }

  lastState = currentState;
}

// ---------------- API ----------------

void createCart() {
  HTTPClient http;

  http.begin(BASE_URL + "/cart/create");
  http.addHeader("Content-Type", "application/json");

  int code = http.POST("{}");

  Serial.print("HTTP Code: ");
  Serial.println(code);

  if (code == 200) {
    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(256);
    deserializeJson(doc, payload);

    cartId = doc["cartId"].as<String>();

    Serial.println("Cart ID: " + cartId);

    String url = "https://smart-cart-174a0.web.app/?cartId=" + cartId;

    showQR(url);

  } else {
    Serial.println("Failed to create cart");
  }

  http.end();
}

// ---------------- QR DISPLAY ----------------

void showQR(String data) {
  display.clear();

  Serial.println("Showing QR:");
  Serial.println(data);

  qr.create(data.c_str());
}

// ---------------- WIFI ----------------

void connectWiFi() {
  Serial.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
}