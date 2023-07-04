#include <SoftwareSerial.h>

#include "SerialBusMaster.h"
#include "MSG_TYPES.h"

#define DEBUG_

#define MSG_TIME_WAIT 180

#define RX_PIN 4  // D2
#define TX_PIN 0  // D3

SoftwareSerial mySerial(RX_PIN, TX_PIN);  // RX, TX (2, 3)

#define WD_PIN 5  // D1

SerialBusMaster bus(&mySerial, 1, 100);
char* buffer = nullptr;

std::vector<uint8_t> nodes;
uint8_t freeAddr = 2;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("ESP started");

  buffer = new char[100];

  nodes.reserve(10);

  pinMode(WD_PIN, OUTPUT);

  digitalWrite(WD_PIN, LOW);

  while (true) {  // дальше не идем пока не подключимся к 1 ноде

    if (waitMessage()) {  // получили ответ от ноды

      Serial.println("msg get");
      if (assignAddress()) {  // попытка дать адрес

        digitalWrite(WD_PIN, HIGH);
        break;  // успешно - выходим
      }
    }
  }
  Serial.println("first node find");
}

void loop() {
  delay(100);
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

bool assignAddress() {
  Serial.println("assignAddress");
  if (bus.getData()[0] == static_cast<char>(MSG_TYPES::GET_ADDR)) {
    buffer[0] = static_cast<char>(MSG_TYPES::SET_ADDR);
    buffer[1] = freeAddr;
    bus.send(BROADCAST_ADDR, buffer, 2);
    Serial.println("sendAddr");
    if (!waitMessage()) { return false; }
    if (bus.getData()[0] == static_cast<char>(MSG_TYPES::PONG) && bus.getTXaddress() == freeAddr) {
      Serial.print("Add new node with address -> ");
      Serial.println(freeAddr);
      nodes.push_back(freeAddr++);
      return true;
    }
  }

  return false;
}