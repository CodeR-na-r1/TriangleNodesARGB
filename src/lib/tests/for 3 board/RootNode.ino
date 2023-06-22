// master with addr == 1

#define DEBUG_

#include "SerialBus.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 0);  // RX, TX

SerialBus bus(&mySerial, 1, 50);

char buffer[50];

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

bool flag = false;

void loop() {
  if (!flag) {

    test0();  // check crc

    delay(500);
    test1();
    Serial.println("Test1 (address 2) completed");

    delay(500);
    test2();
    Serial.println("Test2 (address 3) completed");

    delay(500);
    test3();
    Serial.println("Test3 (broadcast) completed");

    delay(500);
    test4();
    Serial.println("Test4 (sendCommit on 2 adress) completed");

    delay(500);
    Serial.println("Start test 5");
    test5();
    Serial.println("Test5 (send 2 messages (adrr = 2) on 1 adrress with delay) completed");
    // todo add test for buffer

    Serial.print("Tests finished!");

    flag = true;
  }
}

void test0() {
  char bf[10] = { 3, 12, 13, 0, 5, 1, 2, 3, 4, 5 };
  Serial.print("CRC = ");
  Serial.println(bus.crc8(bf, 10));
}

void test1() {
  String s1 = "TEST_1_ADDRESS_2";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(2, buffer, s1.length());
}

void test2() {
  String s1 = "TEST_2_ADDRESS_3";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(3, buffer, s1.length());
}

void test3() {
  String s1 = "TEST_3_Broadcast";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.sendBroadcast(buffer, s1.length());
}

void test4() {
  String s1 = "TEST_4_ADDRESS_2_WITH_COMMIT";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(2, buffer, s1.length(), true);
}

void test5() {
  String s1 = "TEST_5_ADDRESS_2_MSG_1";
  String s2 = "TEST_5_ADDRESS_2_MSG_2";

  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(2, buffer, s1.length());

  delay(100);

  for (int i = 0; i < s2.length(); ++i) {
    buffer[i] = s2[i];
  }
  bus.send(2, buffer, s2.length());
}