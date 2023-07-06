#include <Ticker.h>  //Ticker Library

// #define DEBUG_

#include "SerialBusSlave.h"
#include "MSG_TYPES.h"
#include "ARGB_MODES.h"
#include "LedManager.h"

#define MSG_TIME_WAIT 300
#define MSG_SEND_INTERVAL 200
const uint32_t MULTIPLIER_TIMEOUT_PING = 4000;
uint32_t TIMEOUT_PING = MULTIPLIER_TIMEOUT_PING * 1;

#define FREQUENCY_UPDATE_LED_FOR_LOADING 70

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

// void (*animationFunction)() = nullptr;

Ticker ticker;

bool addrSuccess = false;
auto timerPingResponse = millis();
auto timerLastPing = millis();

ColorRGB color = ColorRGB(59, 26, 93);
ColorRGB colorBackground = ColorRGB(99, 255, 99);
ARGB_MODES mode = ARGB_MODES::STATIC_COLOR;
uint8_t timeDelayAnimation = 60;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP started");

  pinMode(WDP_PIN, INPUT_PULLUP);
  pinMode(WDC0_PIN, OUTPUT);
  pinMode(WDC1_PIN, OUTPUT);

  digitalWrite(WDC0_PIN, HIGH);
  digitalWrite(WDC1_PIN, HIGH);

  // SET LOADING STATIC COLOR
  LED_MANAGER::setBrightness(80);
  color = ColorRGB(255, 255, 255);
  reInitAnimation();

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

  LED_MANAGER::setBrightness(255);
  mode = ARGB_MODES::ANIMATION;
  LED_MANAGER::initLoadAnimation(color, colorBackground, 0, 3, 1);
  timeDelayAnimation = 240;
  reInitAnimation();

  timerPingResponse = millis();
  timerLastPing = millis();
}

void loop() {

  if (bus.available()) {

    char cmd = bus.getData()[0];

    switch (cmd) {

      case static_cast<char>(MSG_TYPES::PING):
        buffer[0] = static_cast<char>(MSG_TYPES::PONG);
        bus.send(1, buffer, 1);
        break;

      case static_cast<char>(MSG_TYPES::SET_TIMEOUT_PING):
        TIMEOUT_PING = MULTIPLIER_TIMEOUT_PING * bus.getData()[1];
        timerLastPing = millis();
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

      case static_cast<char>(MSG_TYPES::BRIGHTNESS):
        LED_MANAGER::setBrightness(bus.getData()[1]);
        break;

      case static_cast<char>(MSG_TYPES::LED_COLOR_INFO):
        color = ColorRGB(bus.getData()[1], bus.getData()[2], bus.getData()[3]);
        break;

      case static_cast<char>(MSG_TYPES::LED_MODE_INFO):
        mode = static_cast<ARGB_MODES>(bus.getData()[1]);
        reInitAnimation();
        break;
    }

    Serial.print("getRXaddress=");
    Serial.println(bus.getRXaddress());
    Serial.print("getAddress=");
    Serial.println(bus.getAddress());
    if (bus.getRXaddress() == bus.getAddress()) {
      Serial.print("update timerLastPing");
      timerLastPing = millis();
    }
  }

  if ((millis() - timerLastPing > TIMEOUT_PING) && addrSuccess) {

    addrSuccess = false;
    Serial.print("Addr lost!");
    // TODO SET RECONECT ANIMATION
    LED_MANAGER::initLoadAnimation(ColorRGB(255, 0, 0), ColorRGB(0, 0, 0), 0, 3, 1);
    mode = ARGB_MODES::ANIMATION;
    timeDelayAnimation = 80;
    reInitAnimation();
  }

  if (isPing()) {

    if (addrSuccess) {

      if (millis() - timerPingResponse > MSG_SEND_INTERVAL) {

        buffer[0] = static_cast<char>(MSG_TYPES::OK);
        bus.send(1, buffer, 1);
        timerPingResponse = millis();
      }
    } else {

      if (millis() - timerPingResponse > MSG_SEND_INTERVAL) {

        if (getAddr()) {  // пробуем получить адрес

          addrSuccess = true;
          Serial.print("Addr restored!");
          // SET DEFAULT ANIMATION
          mode = ARGB_MODES::ANIMATION;
          LED_MANAGER::initLoadAnimation(color, colorBackground, 0, 3, 1);
          timeDelayAnimation = 240;
          reInitAnimation();
        }
        timerPingResponse = millis();
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

    // Serial.println("WDP is active");

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

void reInitAnimation() {

  ticker.detach();

  switch (static_cast<int>(mode)) {

    case static_cast<int>(ARGB_MODES::STATIC_COLOR):
      LED_MANAGER::showColor(color);
      break;

    case static_cast<int>(ARGB_MODES::STATIC_COLOR_ANIM):
      LED_MANAGER::ledFunction = LED_MANAGER::tickStaticColor;
      ticker.attach_ms(timeDelayAnimation, interruptFunction);
      break;

    case static_cast<int>(ARGB_MODES::ANIMATION):
      LED_MANAGER::ledFunction = LED_MANAGER::tickAnimation;
      ticker.attach_ms(timeDelayAnimation, interruptFunction);
      break;

    case static_cast<int>(ARGB_MODES::RAINBOW_ANIM):
      LED_MANAGER::ledFunction = LED_MANAGER::rainbow_fade;
      // timeDelayAnimation = 20;
      ticker.attach_ms(20, interruptFunction);
      break;
  }
}

IRAM_ATTR void interruptFunction() {

  LED_MANAGER::ledFunction();
}