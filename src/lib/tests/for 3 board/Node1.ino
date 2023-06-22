// slave

#define DEBUG_

#include "SerialBus.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);  // RX, TX

SerialBus bus(&mySerial, 2);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

bool flag = false;

void loop() {
  if (!flag) {

    test0();

    test1();
    Serial.println("Test 1 complete");

    test2();
    Serial.println("Test 2 complete");

    test3();
    Serial.println("Test 3 complete");
    
    flag = true;
  }
}

void test0(){
    char bf[10] = { 3, 12, 13, 0, 5, 1, 2, 3, 4, 5 };
    Serial.print("CRC = ");
    Serial.println(bus.crc8(bf, 10));
}

void test1() {
  while (!bus.available()) {
    delay(1);
  }

  bus.getData()[bus.getSizeData()] = '\0';
  Serial.println(String((const char *)bus.getData()));
}

void test2() {
  while (!bus.available()) {
    delay(1);
  }

  bus.getData()[bus.getSizeData()] = '\0';
  Serial.println(String((const char *)bus.getData()));
}

void test3() {
  bus.changeAddress(3);

  while (!bus.available()) {
    delay(1);
  }

  bus.getData()[bus.getSizeData()] = '\0';
  Serial.println(String((const char *)bus.getData()));
}