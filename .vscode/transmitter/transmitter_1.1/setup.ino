#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6
#define SWITCH1_PIN 1
#define SWITCH2_PIN 2
#define LED1_PIN 3
#define LED2_PIN 4
#define HEARTBEAT_INTERVAL 1000 // Send a heartbeat every 1 seconds

bool previousSwitch1State = HIGH;
bool previousSwitch2State = HIGH;
unsigned long lastHeartbeatTime = 0;

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

  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  Serial.println("LoRa init succeeded.");
}

void loop() {
  bool switch1State = digitalRead(SWITCH1_PIN) == LOW; // Invert the switch state
  bool switch2State = digitalRead(SWITCH2_PIN) == LOW; // Invert the switch state

  digitalWrite(LED1_PIN, switch1State ? HIGH : LOW); // Update LED1 state
  digitalWrite(LED2_PIN, switch2State ? HIGH : LOW); // Update LED2 state

  if (switch1State != previousSwitch1State || switch2State != previousSwitch2State) {
    String message = "Switch 1: " + String(switch1State ? "closed" : "open") + ", Switch 2: " + String(switch2State ? "closed" : "open");
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    Serial.println("Sent message: " + message); // Print the sent message
    previousSwitch1State = switch1State;
    previousSwitch2State = switch2State;
  }

  // Send a heartbeat message if it's time
  if (millis() - lastHeartbeatTime >= HEARTBEAT_INTERVAL) {
    LoRa.beginPacket();
    LoRa.print("Heartbeat");
    LoRa.endPacket();
    Serial.println("Sent heartbeat");
    lastHeartbeatTime = millis();
  }

  // Check for incoming messages
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      char c = (char)LoRa.read();
      received += c;
    }

    received.trim(); // Trim the received message
    Serial.println("Received message: " + received);

    // If a request for the switch state was received, send the current switch state
    if (received == "Request switch state") {
      String message = "Switch 1: " + String(switch1State ? "closed" : "open") + ", Switch 2: " + String(switch2State ? "closed" : "open");
      LoRa.beginPacket();
      LoRa.print(message);
      LoRa.endPacket();
      Serial.println("Sent message: " + message); // Print the sent message
    }
  }
}