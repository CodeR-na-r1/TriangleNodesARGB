#include "SerialBus.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);  // RX, TX

SerialBus bus(&mySerial, 0);

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop()
{
  Serial.println(bus.getPacketSize());
}