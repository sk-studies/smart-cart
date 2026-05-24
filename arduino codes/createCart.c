#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "qrcode.h"

#define BUTTON_PIN 4

const char* ssid = "SS2";
const char* password = "SamSwap1603";

const String BASE_URL = "https://api-ktoxqz34xq-el.a.run.app";

String cartId = "";

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  connectWiFi();

  Serial.println("Ready. Press button to create cart...");
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
    delay(200); // debounce

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
    Serial.println("Response:");
    Serial.println(payload);

    DynamicJsonDocument doc(256);
    deserializeJson(doc, payload);

    cartId = doc["cartId"].as<String>();

    Serial.println("Cart Created Successfully!");
    Serial.println("Cart ID: " + cartId);

    // 🔥 Generate QR
    String url = "https://smart-cart-174a0.web.app/?cartId=" + cartId;

    Serial.println("QR URL:");
    Serial.println(url);

    generateQR(url);
    } else {
    Serial.println("Failed to create cart");
  }

  http.end();
}

void generateQR(String data) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];

  qrcode_initText(&qrcode, qrcodeData, 3, 0, data.c_str());

  Serial.println("\nScan this QR:\n");

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      Serial.print(qrcode_getModule(&qrcode, x, y) ? "##" : "  ");
    }
    Serial.println();
  }
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