#include <Wire.h>
#include <SSD1306.h>
#include <qrcodeoled.h>

// OLED I2C
// SDA -> GPIO 21
// SCL -> GPIO 22

SSD1306 display(0x3c, 21, 22);

// QR object
QRcodeOled qrcode(&display);

void setup() {

  Serial.begin(115200);

  // Init display
  display.init();

  // Optional: flip vertically if screen upside down
  // display.flipScreenVertically();

  // Clear screen
  display.clear();
  display.display();

  Serial.println("Generating QR...");

  // Init QR renderer
  qrcode.init();

  // Create QR
  qrcode.create("https://smart-cart-174a0.web.app/?cartId=1233");

  Serial.println("QR displayed");
}

void loop() {
}