#include <Ticker.h>  //Ticker Library

#include "SerialBusSlave.h"
#include "MSG_TYPES.h"

#define DEBUG_

#define MSG_TIME_WAIT 300
#define TIMEOUT_PING 25000

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

bool addrSuccess = false;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP started");

  pinMode(WDP_PIN, INPUT_PULLUP);
  pinMode(WDC0_PIN, OUTPUT);
  pinMode(WDC1_PIN, OUTPUT);

  digitalWrite(WDC0_PIN, HIGH);
  digitalWrite(WDC1_PIN, HIGH);

  // TODO SET LOADING ANIMATION

  while (true) {  // дальше не идем пока не получим адрес

    if (isPing()) {  // если нас опрашивают

      if (getAddr()) {  // пробуем получить адрес

        addrSuccess = true;
        // TODO SET RAINBOW ANIMATION
        break;
      }
    } else {
      bus.clearBuffer();  // пока мы ждем,
                          // возможно сейчас присвают адрес другим slave при помощи широковещательного сообщения - нам это не нужно,
                          // поэтому чистим буфер
    }
  }

  Serial.println("ESP ready");
  Serial.print("Addr =");
  Serial.println(bus.getAddress());
}

void loop() {

  auto static timerLastPing = millis();

  if (bus.available()) {

    char cmd = bus.getData()[0];

    switch (cmd) {

      case static_cast<char>(MSG_TYPES::PING):
        buffer[0] = static_cast<char>(MSG_TYPES::PONG);
        bus.send(1, buffer, 1);
        break;

      case static_cast<char>(MSG_TYPES::SET_WD_0):
        digitalWrite(WDC0_PIN, LOW);
        break;

      case static_cast<char>(MSG_TYPES::SET_WD_1):
        digitalWrite(WDC1_PIN, LOW);
        break;

      case static_cast<char>(MSG_TYPES::RESET_WDs):
        digitalWrite(WDC0_PIN, HIGH);
        digitalWrite(WDC1_PIN, HIGH);
        break;
    }

    if (bus.getRXaddress() == bus.getAddress()) {
      timerLastPing = millis();
    }
  }

  if (millis() - timerLastPing > TIMEOUT_PING) {

    addrSuccess = false;
    // TODO SET RECONECT ANIMATION
  }

  if (isPing()) {

    if (addrSuccess) {

      buffer[0] = static_cast<char>(MSG_TYPES::OK);
      bus.send(1, buffer, 1);
    } else {

      if (getAddr()) {  // пробуем получить адрес

        addrSuccess = true;
        // TODO SET RAINBOW ANIMATION
      }
    }
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

bool isPing() {

  if (!digitalRead(WDP_PIN)) {  // если нас опрашивают

    Serial.println("WDP is active");

    auto timer = millis();
    bool flagSuccessful = false;

    while (!digitalRead(WDP_PIN)) {  // проверяем некоторое время, что сигнал не пропал (защита от помех)

      if (millis() - timer > 20) {

        flagSuccessful = true;
        break;
      }
      delay(1);
    }

    return flagSuccessful;
  }

  return false;
}

bool getAddr() {

  Serial.println("Try get addr -> send broadcast");

  bus.clearBuffer();

  buffer[0] = static_cast<char>(MSG_TYPES::GET_ADDR);  // говорим что нам нужен адрес
  bus.send(1, buffer, 1);

  if (!waitMessage()) {  // не дождались ответа
    Serial.println("!ERROR Try get addr -> no msg");
    return false;
  }

  if (bus.getData()[0] != static_cast<char>(MSG_TYPES::SET_ADDR)) {  // не тот тип ответа
    Serial.println("!ERROR Try get addr -> miss type msg (!= ::SET_ADDR)");
    return false;
  }

  uint8_t newAddr = bus.getData()[1];  // если до сюда дошли, значит мы получили адрес
  bus.changeAddress(newAddr);          // меняем свой адрес

  buffer[0] = static_cast<char>(MSG_TYPES::PONG);  // отмечаемся
  bus.send(1, buffer, 1);

  return true;
}