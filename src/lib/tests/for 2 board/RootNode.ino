// master

#define DEBUG_

#include "SerialBus.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 0);  // RX, TX

SerialBus bus(&mySerial, 1);

char buffer[30];

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

bool flag = false;

void loop() {
  if (!flag) {

    test0();

    delay(500);
    test1();

    delay(500);
    test2();
    
    delay(500);
    test3(); 

    Serial.print("Tests finished!");

    flag = true;
  }
}

void test0(){
    char bf[10] = { 3, 12, 13, 0, 5, 1, 2, 3, 4, 5 };
    Serial.print("CRC = ");
    Serial.println(bus.crc8(bf, 10));
}

void test1() {
  String s1 = "TEST_1";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(2, buffer, s1.length());
    Serial.println("send Test 1");
}

void test2() {
  String s1 = "TEST_2_Broadcast";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.sendBroadcast(buffer, s1.length());
    Serial.println("sendBroadcast");
}

void test3() {
  String s1 = "TEST_3_CHANGE_ADDRESS";
  for (int i = 0; i < s1.length(); ++i) {
    buffer[i] = s1[i];
  }
  bus.send(3, buffer, s1.length());
    Serial.println("send Test 3");
}