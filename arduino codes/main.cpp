#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SSD1306.h>
#include <qrcodeoled.h>
#include <SPI.h>
#include <MFRC522.h>
#include "HX711.h"
#include <map>

// ---------------- CONFIG ----------------

#define BUTTON_PIN 4

#define SS_PIN 5
#define RST_PIN 27

#define RED_LED 2
#define GREEN_LED 14
#define BUZZER 15

#define HX_DOUT 32
#define HX_SCK 33

// ---------------- WIFI ----------------

const char* ssid = "SS2";
const char* password = "SamSwap1603";
const String BASE_URL = "https://api-ktoxqz34xq-el.a.run.app";

// ---------------- OBJECTS ----------------

SSD1306 display(0x3c, 21, 22);
QRcodeOled qr(&display);

MFRC522 rfid(SS_PIN, RST_PIN);
HX711 scale;

String cartId = "";

std::map<String, int> productCount;

unsigned long lastScanTime = 0;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Booting System");

  // ---------------- GPIO CHECK ----------------
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // ---------------- OLED ----------------
  if (!display.init()) {
    Serial.println("❌ OLED init failed");
  }

  display.clear();
  display.drawString(0, 10, "OLED OK");
  display.display();

  qr.init();

  // ---------------- RFID ----------------
  SPI.begin();
  rfid.PCD_Init();

  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);

  if (version == 0x00 || version == 0xFF) {
    Serial.println("❌ RFID not detected");
    displayError("RFID FAIL");
  }

  Serial.println("RFID OK");

  // ---------------- LOAD CELL ----------------
  scale.begin(HX_DOUT, HX_SCK);

  unsigned long start = millis();
  while (!scale.is_ready()) {
    if (millis() - start > 3000) {
      Serial.println("❌ HX711 not found");
      displayError("LOADCELL FAIL");
    }
    delay(200);
  }

  Serial.println("Load Cell OK");

  scale.set_scale(712160);   // your calibrated value
  scale.tare();

  // ---------------- WIFI ----------------
  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);

  unsigned long wifiStart = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - wifiStart > 10000) {
      Serial.println("\n❌ WiFi Failed");
      displayError("WIFI FAIL");
    }

    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");

  // ---------------- FINAL SUCCESS ----------------
  display.clear();
  display.drawString(0, 10, "System Ready");
  display.display();


  Serial.println("System Ready");
}

// ---------------- LOOP ----------------

void loop() {
  handleButton();
  handleRFID();
}

// ---------------- BUTTON ----------------

void handleButton() {
  static bool lastState = HIGH;
  bool currentState = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW) {
    delay(300);

    Serial.println("Reset + Create Cart");

    resetCart();
    createCart();
  }

  lastState = currentState;
}

// ---------------- CART ----------------

void createCart() {
  HTTPClient http;

  http.begin(BASE_URL + "/cart/create");
  http.addHeader("Content-Type", "application/json");

  int code = http.POST("{}");

  if (code == 200) {
    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(256);
    deserializeJson(doc, payload);

    cartId = doc["cartId"].as<String>();

    String url = "https://smart-cart-174a0.web.app/?cartId=" + cartId;

    showQR(url);
  } else {
    errorFeedback();
  }

  http.end();
}

void resetCart() {
  cartId = "";
  productCount.clear();
  scale.tare();

  display.clear();
  display.drawString(0, 20, "Cart Empty!");
  display.display();
}

// ---------------- QR ----------------

void showQR(String data) {
  display.clear();
  qr.create(data.c_str());
}

// ---------------- RFID ----------------

void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  if (millis() - lastScanTime < 3000) return;

  lastScanTime = millis();

  String tag = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    tag += String(rfid.uid.uidByte[i], HEX);
  }

  tag.toUpperCase();

  Serial.println("RFID: " + tag);

  processScan(tag);
}

// ---------------- SCAN ----------------

void processScan(String rfidTag) {
  if (cartId == "") {
    Serial.println("No cart!");
    errorFeedback();
    return;
  }

  HTTPClient http;
  http.begin(BASE_URL + "/cart/scan");
  http.addHeader("Content-Type", "application/json");

  String body = "{\"cartId\":\"" + cartId + "\",\"rfid\":\"" + rfidTag + "\"}";

  int code = http.POST(body);

  if (code == 200) {
    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(512);
    deserializeJson(doc, payload);

    float productWeight = doc["product"]["weight"];

    productCount[rfidTag]++;
    int count = productCount[rfidTag];

    float expected = productWeight * count;

    validateWeight(productWeight);

  } else {
    errorFeedback();
  }

  http.end();
}

// ---------------- WEIGHT ----------------

void validateWeight(float productWeight) {
  float start = scale.get_units(5);

  unsigned long startTime = millis();

  while (millis() - startTime < 3000) {
    float current = scale.get_units(5);

    if ((current - start) >= (productWeight - 0.03)) {
      successFeedback();
      return;
    }
  }

  errorFeedback();
}

// ---------------- FEEDBACK ----------------

void successFeedback() {
  digitalWrite(GREEN_LED, HIGH);
  delay(800);
  digitalWrite(GREEN_LED, LOW);
}

void errorFeedback() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(800);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
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

void displayError(String msg) {
  display.clear();
  display.drawString(0, 10, msg);
  display.display();
}