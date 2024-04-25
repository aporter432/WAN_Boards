#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6

#define SWITCH1_PIN 1
#define SWITCH2_PIN 2
#define SWITCH1_LED_PIN 3
#define SWITCH2_LED_PIN 4

volatile bool switch1State = false;
volatile bool switch2State = false;

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
  pinMode(SWITCH1_LED_PIN, OUTPUT);
  pinMode(SWITCH2_LED_PIN, OUTPUT);

  Serial.println("LoRa init succeeded.");
}

void loop() {
  bool newSwitch1State = !digitalRead(SWITCH1_PIN); // Inverted logic
  bool newSwitch2State = !digitalRead(SWITCH2_PIN); // Inverted logic

  if (newSwitch1State != switch1State) {
    switch1State = newSwitch1State;
    sendSwitchState("Switch 1", switch1State);
    digitalWrite(SWITCH1_LED_PIN, switch1State ? HIGH : LOW); // Update LED status
  }

  if (newSwitch2State != switch2State) {
    switch2State = newSwitch2State;
    sendSwitchState("Switch 2", switch2State);
    digitalWrite(SWITCH2_LED_PIN, switch2State ? HIGH : LOW); // Update LED status
  }

  // Check for incoming LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      char c = (char)LoRa.read();
      received += c;
    }

    // If a "requestSwitchStates" message is received, send the current switch states
    if (received == "requestSwitchStates") {
      sendSwitchState("Switch 1", switch1State);
      digitalWrite(SWITCH1_LED_PIN, switch1State ? HIGH : LOW); // Update LED status
      sendSwitchState("Switch 2", switch2State);
      digitalWrite(SWITCH2_LED_PIN, switch2State ? HIGH : LOW); // Update LED status
    }
  }
}

void sendSwitchState(const char* switchName, bool switchState) {
  LoRa.beginPacket();
  LoRa.print(switchName);
  LoRa.print(": ");
  LoRa.print(switchState ? "closed" : "open");
  LoRa.endPacket();
}