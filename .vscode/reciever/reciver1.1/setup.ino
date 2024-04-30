#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6
#define RELAY1_PIN 1
#define RELAY2_PIN 2
#define LED1_PIN 3
#define LED2_PIN 4
#define HEARTBEAT_TIMEOUT 2000 // 30 seconds, adjust as needed

unsigned long lastHeartbeatTime = 0;
bool communicationLost = true;
bool currentRelay1State = false;
bool currentRelay2State = false;

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

  delay(1500); // Wait for 1 second to let the device stabilize

  // Request the current switch state from the transmitter
  String requestMessage = "Request switch state";
  LoRa.beginPacket();
  LoRa.print(requestMessage);
  LoRa.endPacket();
  Serial.println("Sent message: " + requestMessage);
  
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

    // If a heartbeat was received, update the lastHeartbeatTime
    if (received == "Heartbeat") {
      Serial.println("Heartbeat received");
      lastHeartbeatTime = millis();
      if (communicationLost) {
        // If communication was previously lost, re-establish it and set the relay states to their last known values
        digitalWrite(RELAY1_PIN, currentRelay1State ? HIGH : LOW);
        digitalWrite(RELAY2_PIN, currentRelay2State ? HIGH : LOW);
        digitalWrite(LED1_PIN, currentRelay1State ? HIGH : LOW); // Update LED1 state
        digitalWrite(LED2_PIN, currentRelay2State ? HIGH : LOW); // Update LED2 state
        communicationLost = false;
      }
    }
    // If a switch state was received, update the relay states
    else if (received.startsWith("Switch 1:")) {
      bool switch1State = received.indexOf("Switch 1: closed") != -1;
      bool switch2State = received.indexOf("Switch 2: closed") != -1;

      // Only update relay states if the state has changed
      if (switch1State != currentRelay1State) {
        digitalWrite(RELAY1_PIN, switch1State ? HIGH : LOW); // Update relay1 state
        digitalWrite(LED1_PIN, switch1State ? HIGH : LOW); // Update LED1 state
        currentRelay1State = switch1State;
      }

      if (switch2State != currentRelay2State) {
        digitalWrite(RELAY2_PIN, switch2State ? HIGH : LOW); // Update relay2 state
        digitalWrite(LED2_PIN, switch2State ? HIGH : LOW); // Update LED2 state
        currentRelay2State = switch2State;
      }

      Serial.println("Switch states updated");
    }
  }

  // Check if a heartbeat has been received recently
  if (!communicationLost && millis() - lastHeartbeatTime > HEARTBEAT_TIMEOUT) {
    // If not, open both relays and set the communicationLost flag
    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);
    communicationLost = true;
  }
}