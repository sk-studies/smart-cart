#include "HX711.h"

#define DOUT 4
#define SCK 5

HX711 scale;

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing HX711...");

  scale.begin(DOUT, SCK);

  // Wait for HX711
  while (!scale.is_ready()) {
    Serial.println("HX711 not found...");
    delay(500);
  }

  Serial.println("HX711 Ready!");

  scale.set_scale(712160);
  scale.tare();      // zero weight

  Serial.println("Place weight...");
}

void loop() {
  float weight = scale.get_units(10);

  Serial.print("Weight: ");
  Serial.println(weight);

  delay(1000);
}