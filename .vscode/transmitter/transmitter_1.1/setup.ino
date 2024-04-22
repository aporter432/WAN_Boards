#include <SPI.h>      // include libraries
#include <LoRa.h>

#define frequency 915E6  // LoRa Frequency
bool lostCommunication = true; // flag to track if communication has been lost

#define SWITCH1_PIN 1 // Switch 1 pin
#define SWITCH2_PIN 2 // Switch 2 pin
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
  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  Serial.println("LoRa init succeeded.");
}

void loop() {
  // try to parse packet
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