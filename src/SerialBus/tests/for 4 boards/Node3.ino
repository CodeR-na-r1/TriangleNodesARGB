// slave

#define DEBUG_

#include "SerialBusSlave.h"

SerialBusSlave bus(4, 0, 16, 30, 50);  // RX 2 TX 3 Fake 0

void setup() {
  Serial.begin(9600);
}

bool flag = false;
char s[15] = { '3', '0', '_', 's', 'u', 'p', 'e', 'r', 'S', 't', 'r', 'i', 'n', 'g', '!' };

void loop() {
  if (!flag) {

    getData();  // data from test 1

    bus.send(1, s, 15);  // send for getData address == 30

    getData();  // data from test 3 (broadcast)

    flag = true;
  }
}

void getData() {

  while (!bus.available()) {
    delay(1);
  }

  bus.getData()[bus.getSizeData()] = '\0';
  Serial.println(String((const char *)bus.getData()));
  Serial.print("Adress from = ");
  Serial.println(String(bus.getTXaddress()));
}