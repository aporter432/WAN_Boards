#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6
#define RELAY1_PIN 1
#define RELAY2_PIN 2
#define LED1_PIN 3
#define LED2_PIN 4

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

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
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
    Serial.println("Received signal strength: " + String(LoRa.packetRssi()) + " dB"); // Print the RSSI of the received message

    int switch1Index = received.indexOf("Switch 1: ");
    int switch2Index = received.indexOf(", Switch 2: ");

    if (switch1Index != -1 && switch2Index != -1) {
      String switch1StateStr = received.substring(switch1Index + 10, switch2Index); // Extract the switch 1 state
      switch1StateStr.trim(); // Trim the switch state string
      bool relay1State = switch1StateStr == "closed";
      digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);
      digitalWrite(LED1_PIN, relay1State ? HIGH : LOW); // Update LED1 state

      String switch2StateStr = received.substring(switch2Index + 12); // Extract the switch 2 state
      switch2StateStr.trim(); // Trim the switch state string
      bool relay2State = switch2StateStr == "closed";
      digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
      digitalWrite(LED2_PIN, relay2State ? HIGH : LOW); // Update LED2 state
    }
  }
}