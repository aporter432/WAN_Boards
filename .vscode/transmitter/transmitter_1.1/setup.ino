#include <SPI.h>      // include libraries
#include <LoRa.h>

#define frequency 915E6  // LoRa Frequency

bool lostCommunication = true; // flag to track if communication has been lost

volatile bool switch1State = false; // track the last switch state
volatile bool switchStateChanged = false; // flag to indicate a switch state change

volatile bool switch2State = false; // track the last switch state
unsigned long lastSwitch2ChangeTime = 0; // track the last time the switch state changed

#define DEBOUNCE_DELAY 50 // Debounce delay in milliseconds

#define SWITCH1_PIN 1 // Switch 1 pin
#define SWITCH2_PIN 2 // Switch 2 pin
#define LED1_PIN 3 // LED 1 pin
#define LED2_PIN 4 // LED 2 pin

unsigned long lastSwitchChangeTime = 0; // track the last time the switch state changed

void setup() {
  Serial.begin(9600);   // initialize serial
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(frequency)) {  // initialize LoRa
    Serial.println("LoRa init failed. Check your connections.");
    while (true); // if failed, do nothing
  }

  LoRa.setSpreadingFactor(7); // ranges from 6-12, default is 7
  LoRa.setSignalBandwidth(125E3); // ranges from 7.8E3 to 500E3, default is 125E3
  LoRa.setCodingRate4(5); // ranges from 5-8, default is 5

  // Set pin modes
  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  Serial.println("LoRa init succeeded.");
}

void loop() {
  // Read the switch state
  bool newState = digitalRead(SWITCH1_PIN) == LOW; // LOW means the switch is closed

  // If the switch state has changed and enough time has passed to debounce
  if (newState != switch1State && millis() - lastSwitchChangeTime > DEBOUNCE_DELAY) {
    // Update the switch state and the last switch change time
    switch1State = newState;
    lastSwitchChangeTime = millis();

    // If the switch is closed
    if (switch1State) {
      digitalWrite(LED1_PIN, HIGH); // Turn on the LED

      // Send a message
      LoRa.beginPacket();
      LoRa.print("switch1 closed,");
      LoRa.endPacket();

      // Print switch state
      Serial.println("Switch 1 closed");
    } else {
      digitalWrite(LED1_PIN, LOW); // Turn off the LED

      // Print switch state
      Serial.println("Switch 1 opened");
    }
  }

  // Read the switch 2 state
  bool newState2 = digitalRead(SWITCH2_PIN) == LOW; // LOW means the switch is closed

  // If the switch 2 state has changed and enough time has passed to debounce
  if (newState2 != switch2State && millis() - lastSwitch2ChangeTime > DEBOUNCE_DELAY) {
    // Update the switch 2 state and the last switch change time
    switch2State = newState2;
    lastSwitch2ChangeTime = millis();

    // If the switch 2 is closed
    if (switch2State) {
      digitalWrite(LED2_PIN, HIGH); // Turn on the LED

      // Send a message
      LoRa.beginPacket();
      LoRa.print("switch2 closed,");
      LoRa.endPacket();

      // Print switch state
      Serial.println("Switch 2 closed");
    } else {
      digitalWrite(LED2_PIN, LOW); // Turn off the LED

      // Print switch state
      Serial.println("Switch 2 opened");
    }
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    String received = "";
    while (LoRa.available()) {
      char c = (char)LoRa.read();
      if (c == ',') {
        break; // stop reading at comma
      }
      received += c;
    }

    if (received == "ping") {
      // if received message is "ping", send "pong"
      LoRa.beginPacket();
      LoRa.print("pong,");
      LoRa.endPacket();

      // print sent message
      Serial.println("pong,");

      lostCommunication = false; // set lostCommunication to false
    }

    // print received message and RSSI
    Serial.print(received);
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}