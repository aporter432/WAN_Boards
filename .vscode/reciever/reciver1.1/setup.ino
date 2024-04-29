#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6
#define RELAY_PIN 1

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  pinMode(RELAY_PIN, OUTPUT);
  Serial.println("LoRa init succeeded.");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      char c = (char)LoRa.read();
      received += c;
    }

    received.trim(); // Trim the received message
    Serial.println("Received message: " + received);

    if (received.startsWith("Switch 1 state changed: ")) {
      String switchStateStr = received.substring(24); // Extract the switch state
      switchStateStr.trim(); // Trim the switch state string
      bool relayState = switchStateStr == "closed";
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    }
  }
}