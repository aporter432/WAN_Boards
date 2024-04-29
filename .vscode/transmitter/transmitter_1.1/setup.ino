#include <SPI.h>
#include <LoRa.h>

#define frequency 915E6
#define SWITCH1_PIN 1
#define SWITCH2_PIN 2

bool previousSwitch1State = HIGH;
bool previousSwitch2State = HIGH;

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
  Serial.println("LoRa init succeeded.");
}

void loop() {
  bool switch1State = digitalRead(SWITCH1_PIN) == LOW; // Invert the switch state
  bool switch2State = digitalRead(SWITCH2_PIN) == LOW; // Invert the switch state

  if (switch1State != previousSwitch1State || switch2State != previousSwitch2State) {
    String message = "Switch 1: " + String(switch1State ? "closed" : "open") + ", Switch 2: " + String(switch2State ? "closed" : "open");
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    Serial.println("Sent message: " + message); // Print the sent message
    previousSwitch1State = switch1State;
    previousSwitch2State = switch2State;
  }
}