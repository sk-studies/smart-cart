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
#define END_BUTTON 25

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
std::map<String, float> productPrice;
std::map<String, float> productWeightMap;

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
  pinMode(END_BUTTON, INPUT_PULLUP);

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
  handleEndButton();
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

void handleEndButton() {
  static bool lastState = HIGH;
  bool currentState = digitalRead(END_BUTTON);

  if (lastState == HIGH && currentState == LOW) {
    delay(300);

    Serial.println("Checkout pressed");

    if (!isWeightCorrect()) {
      Serial.println("Weight mismatch!");

      display.clear();
      display.drawString(0, 10, "Weight Error!");
      display.drawString(0, 30, "Check Items");
      display.display();

      errorFeedback("Weight mismatch");
      return;
    }

    Serial.println("Weight OK");

    showPaymentQR();
  }

  lastState = currentState;
}

void showPaymentQR() {
  float totalAmount = calculateTotalAmount();

  Serial.print("Total Amount: ");
  Serial.println(totalAmount);

  // Step 1: Show text
  display.clear();

  display.drawString(0, 10, "Make Payment");
  display.drawString(0, 30, "Rs: " + String(totalAmount, 2));
  display.display();

  Serial.println("Showing Make Payment screen");

  delay(2000); // ⏱️ 2 seconds

  // Step 2: Show QR
  display.clear();

  String upi = "upi://pay?pa=9623058529@ybl&pn=SmartCart&am=" 
             + String(totalAmount, 2) + 
             "&cu=INR";

  Serial.println("Showing UPI QR:");
  Serial.println(upi);

  qr.create(upi.c_str());
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
    Serial.println("Cart URL:");
    Serial.println(url);
  
    showQR(url);
  } else {
    errorFeedback("Invalid Response for the cart creation API");
  }

  http.end();
}

void resetCart() {
  cartId = "";
  productCount.clear();
  productWeightMap.clear();
  productPrice.clear();
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
  successFeedback();

  processScan(tag);
}

// ---------------- SCAN ----------------

void processScan(String rfidTag) {
  if (cartId == "") {
    Serial.println("No cart!");
    errorFeedback("Not Valid Cart!");
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
    String productName = doc["product"]["name"];
    float productPriceValue = doc["product"]["price"];

    productPrice[rfidTag] = productPriceValue;
    productWeightMap[rfidTag] = productWeight;
    productCount[rfidTag]++;
    int count = productCount[rfidTag];

    float expected = productWeight * count;

    showProductAdded(productName);

    validateWeight(productWeight);

  } else {
    errorFeedback("Invalid API response for the RFID Scan");
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

  errorFeedback("Invalid Weight!");
}

bool isWeightCorrect() {
  float expected = calculateExpectedWeight();
  float actual = getCurrentWeight();

  Serial.print("Expected: ");
  Serial.println(expected);

  Serial.print("Actual: ");
  Serial.println(actual);

  float tolerance = 0.15; // 150 grams

  return abs(actual - expected) <= tolerance;
}

float calculateExpectedWeight() {
  float total = 0;

  for (auto &p : productCount) {
    String tag = p.first;
    int qty = p.second;

    float weight = productWeightMap[tag];

    total += weight * qty;
  }

  return total;
}

float getCurrentWeight() {
  return scale.get_units(10); // average
}

float calculateTotalAmount() {
  float total = 0;

  for (auto &p : productCount) {
    String tag = p.first;
    int qty = p.second;

    float price = productPrice[tag];

    total += price * qty;
  }

  return total;
}

// ---------------- FEEDBACK ----------------

void successFeedback() {
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(800);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BUZZER, LOW);
}

void errorFeedback(String msg) {
  Serial.print("Error: " + msg);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(2000);
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

void showProductAdded(String name) {
  display.clear();

  display.drawString(0, 10, "Added:");
  display.drawString(0, 30, name);

  display.display();

  delay(2000);
}