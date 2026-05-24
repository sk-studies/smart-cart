#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "HX711.h"
#include <ArduinoJson.h>
#include <map>

// ---------------- CONFIG ----------------

#define SS_PIN 5
#define RST_PIN 22

#define BUTTON_PIN 4
#define BUZZER 15
#define RED_LED 2
#define GREEN_LED 16

#define HX_DOUT 32
#define HX_SCK 33

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASS";

const String BASE_URL = "https://api-ktoxqz34xq-el.a.run.app";

// ---------------- OBJECTS ----------------

MFRC522 rfid(SS_PIN, RST_PIN);
HX711 scale;

String cartId = "";
float expectedWeight = 0;
float baseWeight = 0;

std::map<String, int> productCount;

unsigned long lastScanTime = 0;
const int SCAN_DEBOUNCE = 3000;

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  SPI.begin();
  rfid.PCD_Init();

  scale.begin(HX_DOUT, HX_SCK);
  scale.set_scale();     // Calibrate this!
  scale.tare();

  connectWiFi();

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
    delay(800);

    Serial.println("Creating new cart...");
    createCart();

    resetCart();
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
    Serial.println("Cart ID: " + cartId);
  } else {
    Serial.println("Cart creation failed");
    errorBeep();
  }

  http.end();
}

void resetCart() {
  expectedWeight = 0;
  productCount.clear();

  delay(500);
  baseWeight = getWeight();

  Serial.println("Cart Reset. Base Weight: " + String(baseWeight));
}

// ---------------- RFID ----------------

void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  if (millis() - lastScanTime < SCAN_DEBOUNCE) return;

  lastScanTime = millis();

  String tag = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    tag += String(rfid.uid.uidByte[i], HEX);
  }

  tag.toUpperCase();
  Serial.println("Scanned RFID: " + tag);

  processScan(tag);
}

// ---------------- SCAN ----------------

void processScan(String rfidTag) {
  if (cartId == "") {
    Serial.println("Cart not initialized!");
    errorBeep();
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

    float totalExpected = productWeight * count;

    Serial.println("Expected Weight for this product: " + String(totalExpected));

    beepOnce();

    validateWeight(productWeight);

  } else {
    Serial.println("Scan API failed");
    errorBeep();
  }

  http.end();
}

// ---------------- WEIGHT ----------------

float getWeight() {
  return scale.get_units(5);
}

void validateWeight(float productWeight) {
  float startWeight = getWeight();
  unsigned long startTime = millis();

  while (millis() - startTime < 3000) {
    float current = getWeight();

    if ((current - startWeight) >= (productWeight - 0.02)) {
      successLED();
      Serial.println("Weight OK");
      return;
    }
  }

  Serial.println("Weight mismatch!");
  errorBeep();
}

// ---------------- FEEDBACK ----------------

void beepOnce() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
}

void errorBeep() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(300);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }
}

void successLED() {
  digitalWrite(GREEN_LED, HIGH);
  delay(1000);
  digitalWrite(GREEN_LED, LOW);
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