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

void handleSwitchState(String received, volatile bool& switchState, int relayPin, int ledPin) {
  received.trim();
  int closedIndex = received.indexOf("closed");
  int openIndex = received.indexOf("open");

  // Only update the state if the received message contains switch state information
  if (closedIndex != -1 || openIndex != -1) {
    bool receivedState = closedIndex != -1;
    if (receivedState != switchState) {
      switchState = receivedState;
      digitalWrite(relayPin, switchState ? HIGH : LOW);
      digitalWrite(ledPin, switchState ? HIGH : LOW);
    }
  }
}
unsigned long lastBlinkTime = 0;
bool ledState = false;  
void blinkLEDs() {
  if (millis() - lastBlinkTime >= 250) {
    ledState = !ledState;

    digitalWrite(LED1_PIN, ledState ? HIGH : LOW);
    digitalWrite(LED2_PIN, ledState ? LOW : HIGH);

    lastBlinkTime = millis();
  }
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

      // Print the switch states
      Serial.println("Switch 1 state: " + String(switch1State ? "closed" : "open"));
      Serial.println("Switch 2 state: " + String(switch2State ? "closed" : "open"));
    } else if (received.startsWith("Switch 1")) {
      handleSwitchState(received, switch1State, RELAY1_PIN, LED1_PIN);
    } else if (received.startsWith("Switch 2")) {
      handleSwitchState(received, switch2State, RELAY2_PIN, LED2_PIN);
    }
  }

  // Check if the connection is lost
  if (millis() - lastPingTime > PING_TIMEOUT) {
    if (!lostCommunication) {
      Serial.println("Communication lost");
    }
    lostCommunication = true;
  }

  // If communication is lost, blink the LEDs and set both relay pins to LOW
  if (lostCommunication) {
    blinkLEDs();
    digitalWrite(RELAY1_PIN, LOW); // Set relay 1 to LOW
    digitalWrite(RELAY2_PIN, LOW); // Set relay 2 to LOW
  }
}