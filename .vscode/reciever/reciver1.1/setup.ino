#include <SPI.h>      // include libraries
#include <LoRa.h>

#define frequency 915E6  // LoRa Frequency
bool lostCommunication = true; // flag to track if communication has been lost

volatile bool switch1State = false; // false means opened, true means closed
volatile bool switch2State = false; // false means opened, true means closed

#define RELAY1_PIN 1 // Relay 1 pin
#define RELAY2_PIN 2 // Relay 2 pin
#define LED1_PIN 3 // LED 1 pin
#define LED2_PIN 4 // LED 2 pin

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
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  Serial.println("LoRa init succeeded.");
}

bool handleSwitch1State(String received) {
  received.trim(); // remove leading and trailing spaces
  bool previousState = switch1State;
  if (received == "switch1 closed," && !switch1State) {
    switch1State = true;
  } else if (received == "switch1 opened," && switch1State) {
    switch1State = false;
  }
  return previousState != switch1State; // return true if the state was changed
}
bool handleSwitch2State(String received) {
  received.trim(); // remove leading and trailing spaces
  bool previousState = switch2State;
  if (received == "switch2 closed," && !switch2State) {
    switch2State = true;
  } else if (received == "switch2 opened," && switch2State) {
    switch2State = false;
  }
  return previousState != switch2State; // return true if the state was changed
}


void loop() {
  static unsigned long lastPingTime = 0;
  unsigned long currentMillis = millis();

  // send ping every two seconds
  if (millis() - lastPingTime >= 2000) {
    LoRa.beginPacket();
    LoRa.print("ping,");
    LoRa.endPacket();

    Serial.print("Sent: ping, with RSSI ");
    Serial.println(LoRa.packetRssi());

    lastPingTime = millis(); // update last ping time
  }

  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    String received = "";
    while (LoRa.available()) {
      char c = (char)LoRa.read();
      received += c; // append all characters to the received string
    }

    // print received message and RSSI
    Serial.print("Received: ");
    Serial.print(received);
    Serial.print(", with RSSI ");
    Serial.println(LoRa.packetRssi());

    // process the received message
    if (received.startsWith("pong")) {
      lostCommunication = false; // set lostCommunication to false
    } else if (received.startsWith("switch1")) {
      bool switchStateUpdated = handleSwitch1State(received); // handle switch states
      if (switchStateUpdated) {
        digitalWrite(RELAY1_PIN, switch1State ? HIGH : LOW); // update the relay only if switch state was changed
        digitalWrite(LED1_PIN, switch1State ? HIGH : LOW); // update the LED only if switch state was changed
      }
    } else if (received.startsWith("switch2")) {
      bool switchStateUpdated = handleSwitch2State(received); // handle switch states
      if (switchStateUpdated) {
        digitalWrite(RELAY2_PIN, switch2State ? HIGH : LOW); // update the relay only if switch state was changed
        digitalWrite(LED2_PIN, switch2State ? HIGH : LOW); // update the LED only if switch state was changed
      }
    }
  } // This is the missing closing brace
}