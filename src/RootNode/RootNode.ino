#include <SoftwareSerial.h>

// #define DEBUG_

#include "SerialBusMaster.h"
#include "MSG_TYPES.h"

#define MSG_TIME_WAIT 200
#define PING_PONG_FREQUENCY 700

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

auto timerPingPong = millis();
uint8_t nodeIterator = 0;
bool isSecondNeigbour = false;

void loop() {

  if (millis() - timerPingPong > PING_PONG_FREQUENCY) {

    bus.clearBuffer();

    buffer[0] = static_cast<char>(MSG_TYPES::PING);
    bus.send(nodes[nodeIterator], buffer, 1);

    if (waitMessage()) {

      if (bus.getData()[0] == static_cast<char>(MSG_TYPES::PONG)) {

        buffer[0] = static_cast<char>(isSecondNeigbour == false ? MSG_TYPES::SET_WD_0 : MSG_TYPES::SET_WD_1);
        bus.send(nodes[nodeIterator], buffer, 1);

        if (waitMessage()) {  // получили ответ от ноды (либо у которой есть адрес, либо нету)

          if (bus.getData()[0] != static_cast<char>(MSG_TYPES::OK)) {  // (которая отреагировала на watch dog (то есть безАдресная))
            Serial.print("addr  node intent = ");
            Serial.println(nodes[nodeIterator]);
            Serial.print("isSecondNeigbour = ");
            Serial.println(isSecondNeigbour);
            Serial.print("TYPE_MSG  = ");
            Serial.println(static_cast<int>(bus.getData()[0]));
            assignAddress();  // попытка дать адрес
          }
        }

        buffer[0] = static_cast<char>(MSG_TYPES::RESET_WDs);
        bus.send(nodes[nodeIterator], buffer, 1);
      }
    } else {

      Serial.print("Erase node, addres -> ");
      Serial.println(nodes[nodeIterator]);
      Serial.print("; remains -> ");
      nodes.erase(nodes.begin() + nodeIterator);
      Serial.println(nodes.size());

      Serial.println("nodes = ");
      for (int i = 0; i < nodes.size(); ++i) {
        if (i > 0) { Serial.print(", "); }
        Serial.print(nodes[i]);
      }
      Serial.println();

      if (nodes.size() == 0) {
        Serial.println("Nodes nothing. EPS reset!");
        ESP.reset();
      }

      nodeIterator = nodeIterator % nodes.size();
      isSecondNeigbour = false;
    }

    if (isSecondNeigbour == false) {
      isSecondNeigbour = true;
    } else {
      isSecondNeigbour = false;
      ++nodeIterator;
      if (nodeIterator >= nodes.size()) { nodeIterator = 0; }
    }

    timerPingPong = millis();
  }

  // TODO handler for queue msgs from serverCallback or microPhone and others
  // Add packets for led strip commands
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