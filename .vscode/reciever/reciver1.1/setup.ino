#include <SPI.h>      // include libraries
#include <LoRa.h>

#define frequency 915E6  // LoRa Frequency

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

  Serial.println("LoRa init succeeded.");
}


void loop() {
  // send ping
  LoRa.beginPacket();
  LoRa.print("ping,");
  LoRa.endPacket();

  // print sent message
  Serial.println("ping,");

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

    // print received message and RSSI
    Serial.print(received);
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}