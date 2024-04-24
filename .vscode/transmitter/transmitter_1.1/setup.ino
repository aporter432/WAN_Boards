#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6

bool lostCommunication = true;

volatile bool switch1State = false;
volatile bool switchStateChanged = false;

volatile bool switch2State = false;
unsigned long lastSwitch2ChangeTime = 0;

#define DEBOUNCE_DELAY 50

#define SWITCH1_PIN 1
#define SWITCH2_PIN 2
#define LED1_PIN 3
#define LED2_PIN 4

unsigned long lastSwitchChangeTime = 0;

#define PING_INTERVAL 1500 // Send a ping every 5 seconds
#define PING_TIMEOUT  3000 // Consider the connection lost after 10 seconds without a pong

unsigned long lastPingTime = 0;
unsigned long lastPongTime = 0;

void setupLoRa() {
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
}

void setupPins() {
  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Transmitter");

  setupLoRa();
  setupPins();

  // Check initial switch states
  switch1State = digitalRead(SWITCH1_PIN);
  switch2State = digitalRead(SWITCH2_PIN);

  // Update LED states based on initial switch states
  digitalWrite(LED1_PIN, switch1State ? LOW : HIGH);
  digitalWrite(LED2_PIN, switch2State ? LOW : HIGH);

  // Send initial switch states
  handleSwitch(SWITCH1_PIN, switch1State, lastSwitchChangeTime, LED1_PIN, "Switch 1");
  delay(1000); // Add a delay to give the receiver time to process the message
  handleSwitch(SWITCH2_PIN, switch2State, lastSwitchChangeTime, LED2_PIN, "Switch 2");

  Serial.println("LoRa init succeeded.");
}

void handleSwitch(int switchPin, volatile bool& switchState, unsigned long& lastSwitchChangeTime, int ledPin, const char* switchName) {
  bool newState = !digitalRead(switchPin); // Inverted logic for active low switches
  if (newState != switchState && millis() - lastSwitchChangeTime > DEBOUNCE_DELAY) {
    switchState = newState;
    lastSwitchChangeTime = millis();

    String switchStateStr = switchState ? " closed," : " opened,";
    String message = switchName + switchStateStr;

    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();

    digitalWrite(ledPin, switchState ? HIGH : LOW); // Inverted logic for active low LEDs

    Serial.println("Sent: " + message);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
  }
}

void handleReceivedPacket() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      char c = (char)LoRa.read();
      if (c == ',') {
        break;
      }
      received += c;
    }

    if (received == "pong") {
      Serial.println("Received: " + received);
      Serial.println("RSSI: " + String(LoRa.packetRssi()));

      lastPongTime = millis(); // Update the time of the last pong
    }
  }
}

void loop() {
  handleSwitch(SWITCH1_PIN, switch1State, lastSwitchChangeTime, LED1_PIN, "Switch 1");
  handleSwitch(SWITCH2_PIN, switch2State, lastSwitch2ChangeTime, LED2_PIN, "Switch 2");
  handleReceivedPacket();

  // Send a ping if it's time for the next ping
  if (millis() - lastPingTime > PING_INTERVAL) {
    String message = "ping,";
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();

    Serial.println("Sent: " + message);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));

    lastPingTime = millis();
  }

  // Check if the connection is lost
  if (millis() - lastPingTime > PING_TIMEOUT) {
    if (!lostCommunication) {
      Serial.println("Communication lost");
    }
    lostCommunication = true;
  }
}