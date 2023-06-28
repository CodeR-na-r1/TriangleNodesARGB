// master

#define DEBUG_
#define GET_DATA_TIME 5000

#include "SerialBusMaster.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(5, 0);  // RX, TX (1, 3)

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
    test0();
    Serial.println("Test0 (sendCommit on 10 adress) completed");

    getData();
    Serial.println("get data from 10 adress");

    delay(500);
    test1();
    Serial.println("Test1 (sendCommit on 20 adress) completed");

    getData();
    Serial.println("get data from 20 adress");

    delay(500);
    test2();
    Serial.println("Test2 (sendCommit on 30 adress) completed");

    getData();
    Serial.println("get data from 30 adress");

    delay(500);
    test3();
    Serial.println("Test3 (broadcast) completed");

    Serial.print("Tests finished!");

    flag = true;
  }
}

void test0() {
  String s1 = "TEST_4_ADDRESS_10_WITH_COMMIT";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(10, buffer, s1.length(), true);
}

void test1() {
  String s1 = "TEST_4_ADDRESS_20_WITH_COMMIT";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(20, buffer, s1.length(), true);
}

void test2() {
  String s1 = "TEST_4_ADDRESS_30_WITH_COMMIT";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(30, buffer, s1.length(), true);
}

void test3() {
  String s1 = "TEST_3_BROADCAST";

  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.sendBroadcast(buffer, s1.length());
}

void getData() {
  auto timer = millis();
  while (!bus.available()) {
    if (millis() - timer > GET_DATA_TIME) {
      Serial.print("NO DATA TIMER OUT");
      return;
    }
    delay(1);
  }

  bus.getData()[bus.getSizeData()] = '\0';
  Serial.println(String((const char *)bus.getData()));
  Serial.print("Adress from = ");
  Serial.println(String(bus.getTXaddress()));
}