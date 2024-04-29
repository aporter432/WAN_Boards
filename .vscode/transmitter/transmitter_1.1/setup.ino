#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6
#define SWITCH_PIN 1

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Transmitter");

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  Serial.println("LoRa init succeeded.");
}

void loop() {
  bool switchState = digitalRead(SWITCH_PIN) == LOW; // Invert the switch state
  String message = "Switch 1 state changed: " + String(switchState ? "closed" : "open");

  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  Serial.println("Sent message: " + message); // Print the sent message

}