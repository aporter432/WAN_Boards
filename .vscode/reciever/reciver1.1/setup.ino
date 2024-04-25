#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6

volatile bool switch1State = false;
volatile bool switch2State = false;

#define RELAY1_PIN 1
#define RELAY2_PIN 2
#define LED1_PIN 3
#define LED2_PIN 4

void requestSwitchStates() {
  String message = "requestSwitchStates";

  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  Serial.println("Sent: " + message); // Log the actual transmission
}

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
  // Request the switch states from the transmitter
  delay(2000); // Give the LoRa module some time to stabilize
  requestSwitchStates();
}

bool handleSwitchState(String received, volatile bool& switchState) {
  received.trim();
  int closedIndex = received.indexOf("closed");
  int openIndex = received.indexOf("open");

  // Only update the state if the received message contains switch state information
  if (closedIndex != -1 || openIndex != -1) {
    bool receivedState = closedIndex != -1;
    if (receivedState != switchState) {
      switchState = receivedState;
      return true; // Return true to indicate that the switch state was updated
    }
  }

  return false; // Return false to indicate that the switch state was not updated
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

    if (received.startsWith("Switch 1")) {
      if (handleSwitchState(received, switch1State)) {
        // Update the outputs only if the switch state was updated
        digitalWrite(RELAY1_PIN, switch1State ? HIGH : LOW);
        digitalWrite(LED1_PIN, switch1State ? HIGH : LOW);
      }
    } else if (received.startsWith("Switch 2")) {
      if (handleSwitchState(received, switch2State)) {
        // Update the outputs only if the switch state was updated
        digitalWrite(RELAY2_PIN, switch2State ? HIGH : LOW);
        digitalWrite(LED2_PIN, switch2State ? HIGH : LOW);
      }
    }
  }
}