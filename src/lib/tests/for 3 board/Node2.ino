// slave with addr = 3

#define DEBUG_

#include "SerialBus.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);  // RX, TX

SerialBus bus(&mySerial, 3, 50);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

bool flag = false;

void loop() {
  if (!flag) {

    test0();  // check crc

    getData();  // data from test 1 or 2

    getData();  // data from test 3 (broadcast)

    getData();  // data from test 4

    Serial.println("Delay before test 5");
    delay(5000);
    Serial.println("Delay be over test 5");
    getData();  // data from test 5
    getData();  // data from test 5

    flag = true;
  }
}

void test0() {
  char bf[10] = { 3, 12, 13, 0, 5, 1, 2, 3, 4, 5 };
  Serial.print("CRC = ");
  Serial.println(bus.crc8(bf, 10));
}

void getData() {

  while (!bus.available()) {
    delay(1);
  }

  bus.getData()[bus.getSizeData()] = '\0';
  Serial.println(String((const char *)bus.getData()));
}