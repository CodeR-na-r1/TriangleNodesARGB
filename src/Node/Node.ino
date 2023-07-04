#include <Ticker.h>  //Ticker Library

#include "SerialBusSlave.h"
#include "MSG_TYPES.h"

#define DEBUG_

#define MSG_TIME_WAIT 300

#define RX_PIN 4     // D2
#define TX_PIN 0     // D3
#define FAKE_PIN 16  // D0

#define WDP_PIN 2    // D4
#define WDC0_PIN 12  // D6 (right neigbour)
#define WDC1_PIN 14  // D5 (down neigbour)

#define LED_PIN 5  // D1
#define LED_NUM 12

SerialBusSlave bus = SerialBusSlave(RX_PIN, TX_PIN, FAKE_PIN, BROADCAST_ADDR, 100);  // RX 2 TX 3 Fake 0 addr 0 bufSz 100
char* buffer = new char[100];

void setup() {
  Serial.begin(9600);
  Serial.println("ESP started");

  pinMode(WDP_PIN, INPUT_PULLUP);
  pinMode(WDC0_PIN, OUTPUT);
  pinMode(WDC1_PIN, OUTPUT);

  digitalWrite(WDC0_PIN, LOW);
  digitalWrite(WDC1_PIN, LOW);

  while (true) {

    if (!digitalRead(WDP_PIN)) {
      Serial.println("WDP is active");

      auto timer = millis();
      bool flagSuccessful = false;

      while (!digitalRead(WDP_PIN)) {

        if (millis() - timer > 20) {

          flagSuccessful = true;
          break;
        }
        delay(1);
      }

      if (flagSuccessful) {
        Serial.println("Try get addr -> send broadcast");

        bus.clearBuffer();

        buffer[0] = static_cast<char>(MSG_TYPES::GET_ADDR);
        bus.send(1, buffer, 1);

        if (!waitMessage()) {
          Serial.println("!ERROR Try get addr -> no msg");
          continue;
        }

        if (bus.getData()[0] != static_cast<char>(MSG_TYPES::SET_ADDR)) {
          Serial.println("!ERROR Try get addr -> miss type msg (!= ::SET_ADDR)");
          continue;
        }

        uint8_t newAddr = bus.getData()[1];
        bus.changeAddress(newAddr);

        buffer[0] = static_cast<char>(MSG_TYPES::PONG);
        bus.send(1, buffer, 1);

        break;
      }
    } else {
      bus.clearBuffer();
    }
  }

  Serial.println("ESP ready");
  Serial.print("Addr =");
  Serial.println(bus.getAddress());
}

void loop() {
}

bool waitMessage() {
  auto timer = millis();

  while (millis() - timer < MSG_TIME_WAIT) {
    if (bus.available()) {
      break;
    }
  }

  return bus.available();
}