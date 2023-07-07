#include <SoftwareSerial.h>

// #define DEBUG_

#include "SerialBusMaster.h"
#include "MSG_TYPES.h"
#include "ServerModule.h"
#include "ARGB_MODES.h"

#define MSG_TIME_WAIT 250
#define PING_PONG_FREQUENCY 850

#define RX_PIN 4  // D2
#define TX_PIN 0  // D3

SoftwareSerial mySerial(RX_PIN, TX_PIN);  // RX, TX (2, 3)

#define WD_PIN 5  // D1

SerialBusMaster bus(&mySerial, 1, 100);
char* buffer = nullptr;

std::vector<uint8_t> nodes;
uint8_t freeAddr = 2;

auto timerPingPong = millis();

/* address strip parameters */

char brightness = 255;
char ledDelay = 240;
char mode = static_cast<char>(ARGB_MODES::ANIMATION);
char rColor = 23;
char gColor = 8;
char bColor = 59;

char* queueBuffer = new char[20];
uint8_t queueSize = 0;
bool isQueueData = false;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("ESP started");

  if (WIFI_AP_NAMESPACE::start() == 0) {
    Serial.println("WIFI AP created");
  } else {
    Serial.println("Wifi AP created error!");
  }

  int ret = SERVER_NAMESPACE::start();

  if (ret == 0) {
    Serial.println("Server started;");

    SERVER_NAMESPACE::setuserCahngeColorCallback(serverChangeColorCallback);
    SERVER_NAMESPACE::setuserCahngeBrightnessCallback(serverChangeBrightnessCallback);
    SERVER_NAMESPACE::setuserCahngeDelayCallback(serverChangeDelayCallback);
    SERVER_NAMESPACE::setuserCahngeModeCallback(serverChangeModeCallback);
    Serial.println("setUserCallback is set");
  } else {
    Serial.print("Server started, with error, retValue -> ");
    Serial.println(ret);
  }

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

  timerPingPong = millis();
}

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

          Serial.print("after pong (response OK (0) or GET_ADDR(1))\naddr  node intent = ");
          Serial.println(nodes[nodeIterator]);
          Serial.print("isSecondNeigbour = ");
          Serial.println(isSecondNeigbour);
          Serial.print("TYPE_MSG  = ");
          Serial.println(static_cast<int>(bus.getData()[0]));

          if (bus.getData()[0] == static_cast<char>(MSG_TYPES::GET_ADDR)) {  // (которая отреагировала на watch dog (то есть безАдресная))

            if (assignAddress()) {  // попытка дать адрес

              buffer[0] = static_cast<char>(MSG_TYPES::SET_TIMEOUT_PING);
              buffer[1] = nodes.size() & 0xFF;
              bus.sendBroadcast(buffer, 2);

              // send state new node

              sendState(nodes[nodes.size() - 1]);
            }
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

      buffer[0] = static_cast<char>(MSG_TYPES::SET_TIMEOUT_PING);
      buffer[1] = nodes.size() & 0xFF;
      bus.sendBroadcast(buffer, 2);

      nodeIterator = nodeIterator % nodes.size();
      Serial.print("nodeIterator = ");
      Serial.println(nodeIterator);
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

  // handler for queue msgs from serverCallback or microPhone and others

  if (isQueueData) {

    bus.sendBroadcast(queueBuffer, queueSize);
    isQueueData = false;
  }
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

void serverChangeColorCallback(String _r, String _g, String _b) {

  queueBuffer[0] = static_cast<char>(MSG_TYPES::LED_COLOR_INFO);
  queueBuffer[1] = atoi(_r.c_str());
  queueBuffer[2] = atoi(_g.c_str());
  queueBuffer[3] = atoi(_b.c_str());

  isQueueData = true;

  queueSize = 4;

  // save state

  rColor = atoi(_r.c_str());
  gColor = atoi(_g.c_str());
  bColor = atoi(_b.c_str());

  Serial.print("r = ");
  Serial.println(_r);

  Serial.print("g = ");
  Serial.println(_g);

  Serial.print("b = ");
  Serial.println(_b);
}

void serverChangeBrightnessCallback(String _brightness) {

  queueBuffer[0] = static_cast<char>(MSG_TYPES::BRIGHTNESS);
  queueBuffer[1] = atoi(_brightness.c_str());

  isQueueData = true;

  queueSize = 2;

  brightness = atoi(_brightness.c_str());

  Serial.print("brightness = ");
  Serial.println(_brightness);
}

void serverChangeDelayCallback(String _delay) {

  queueBuffer[0] = static_cast<char>(MSG_TYPES::DELAY);
  queueBuffer[1] = atoi(_delay.c_str());

  isQueueData = true;

  queueSize = 2;

  ledDelay = atoi(_delay.c_str());

  Serial.print("delay = ");
  Serial.println(_delay);
}

void serverChangeModeCallback(String _mode) {

  queueBuffer[0] = static_cast<char>(MSG_TYPES::LED_MODE_INFO);

  if (_mode == "Static") {

    queueBuffer[1] = static_cast<char>(ARGB_MODES::STATIC_COLOR);
    mode = static_cast<char>(ARGB_MODES::STATIC_COLOR);

  } else if (_mode == "StaticAnim") {

    queueBuffer[1] = static_cast<char>(ARGB_MODES::STATIC_COLOR_ANIM);
    mode = static_cast<char>(ARGB_MODES::STATIC_COLOR_ANIM);

  } else if (_mode == "Animation") {

    queueBuffer[1] = static_cast<char>(ARGB_MODES::ANIMATION);
    mode = static_cast<char>(ARGB_MODES::ANIMATION);

  } else if (_mode == "Rainbow") {

    queueBuffer[1] = static_cast<char>(ARGB_MODES::RAINBOW_ANIM);
    mode = static_cast<char>(ARGB_MODES::RAINBOW_ANIM);

  } else if (_mode == "ColorBounce") {

    queueBuffer[1] = static_cast<char>(ARGB_MODES::COLOR_BOUNCE_FADE);
    mode = static_cast<char>(ARGB_MODES::COLOR_BOUNCE_FADE);
  }

  isQueueData = true;

  queueSize = 2;

  Serial.print("mode = ");
  Serial.println(_mode);
}

void sendState(uint8_t addr) {

  //send color

  buffer[0] = static_cast<char>(MSG_TYPES::LED_COLOR_INFO);
  buffer[1] = rColor;
  buffer[2] = gColor;
  buffer[3] = bColor;
  bus.send(addr, buffer, 4);

  //send brightness

  buffer[0] = static_cast<char>(MSG_TYPES::BRIGHTNESS);
  buffer[1] = brightness;
  bus.send(addr, buffer, 2);

  //send delay

  buffer[0] = static_cast<char>(MSG_TYPES::DELAY);
  buffer[1] = ledDelay;
  bus.send(addr, buffer, 2);

  //send mode

  buffer[0] = static_cast<char>(MSG_TYPES::LED_MODE_INFO);
  buffer[1] = mode;
  bus.send(addr, buffer, 2);
}