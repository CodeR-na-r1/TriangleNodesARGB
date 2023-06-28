// master

#define DEBUG_

#include "SerialBusMaster.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 0);  // RX, TX (2, 3)

SerialBusMaster bus(&mySerial, 1, 50);

char buffer[50];

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

bool flag = false;

void loop() {
  if (!flag) {

    delay(500);
    test2();
    Serial.println("Test2 (sendCommit on 13 adress) completed");

    getData();
    Serial.println("get data from 13 adress");

    delay(500);
    test3();
    Serial.println("Test3 (broadcast) completed");

    Serial.print("Tests finished!");

    flag = true;
  }
}

void test2() {
  String s1 = "TEST_4_ADDRESS_13_WITH_COMMIT";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(13, buffer, s1.length(), true);
}

void test3() {
  String s1 = "TEST_3_BROADCAST";

  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.sendBroadcast(buffer, s1.length());
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