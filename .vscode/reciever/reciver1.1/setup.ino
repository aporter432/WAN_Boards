#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6
bool lostCommunication = true;

volatile bool switch1State = false;
volatile bool switch2State = false;

#define RELAY1_PIN 1
#define RELAY2_PIN 2
#define LED1_PIN 3
#define LED2_PIN 4

#define PING_TIMEOUT  10000 // Consider the connection lost after 10 seconds without a ping

unsigned long lastPingTime = 0;

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

bool handleSwitch1State(String received) {
  received.trim();
  bool previousState = switch1State;
  if (received.equalsIgnoreCase("Switch 1 closed,") && !switch1State) {
    switch1State = true;
  } else if (received.equalsIgnoreCase("Switch 1 opened,") && switch1State) {
    switch1State = false;
  }
  return previousState != switch1State;
}

bool handleSwitch2State(String received) {
  received.trim();
  bool previousState = switch2State;
  if (received.equalsIgnoreCase("Switch 2 closed,") && !switch2State) {
    switch2State = true;
  } else if (received.equalsIgnoreCase("Switch 2 opened,") && switch2State) {
    switch2State = false;
  }
  return previousState != switch2State;
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      char c = (char)LoRa.read();
      received += c;
    }

    Serial.println("Received: " + received);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));

    if (received.startsWith("ping")) {
      lostCommunication = false;
      lastPingTime = millis();

      // Respond with a pong
      LoRa.beginPacket();
      LoRa.print("pong,");
      LoRa.endPacket();

      Serial.println("Sent: pong");
      Serial.println("RSSI: " + String(LoRa.packetRssi()));
    } else if (received.startsWith("Switch 1")) {
      bool switchStateUpdated = handleSwitch1State(received);
      if (switchStateUpdated) {
        digitalWrite(RELAY1_PIN, switch1State ? HIGH : LOW);
        digitalWrite(LED1_PIN, switch1State ? HIGH : LOW);
      }
    } else if (received.startsWith("Switch 2")) {
      bool switchStateUpdated = handleSwitch2State(received);
      if (switchStateUpdated) {
        digitalWrite(RELAY2_PIN, switch2State ? HIGH : LOW);
        digitalWrite(LED2_PIN, switch2State ? HIGH : LOW);
      }
    }
  }

  // Check if the connection is lost
  if (millis() - lastPingTime > PING_TIMEOUT) {
    lostCommunication = true;
  }
}