/*

  Параметры конструктора:
  TODO

  Структура пакета:
  TODO  

*/


#ifndef SERIAL_BUS_SLAVE
#define SERIAL_BUS_SLAVE

#include <SoftwareSerial.h>

#define SERIAL_SPEED 9600
#define START_SYMBOL 3
#define WAITING_DATA_TIMEOUT 50
#define WAITING_SEND_COMMIT 3000

#define BROADCAST_ADDR 0

class SerialBusSlave {

private:

  SoftwareSerial* stream_;
  SoftwareSerial* stream2_;
  SoftwareSerial* streamTemp_;

  uint8_t pinRX_;
  uint8_t pinTX_;

  bool isRecieveMode_;

  uint8_t address_;
  uint16_t bufferSize_;

  char* buffer_;

  bool isData_ = false;
  uint8_t addrTo_;
  uint8_t addrFrom_;
  uint16_t sizeData_;

  uint16_t errorCounter_ = 0;
  static constexpr uint8_t MIN_PACKET_SIZE_ = 1 + sizeof(address_) + sizeof(addrFrom_) + sizeof(sizeData_) + sizeof(char) + sizeof(int8_t) + sizeof(int8_t);

public:

  SerialBusSlave(const uint8_t pinRX, const uint8_t pinTX, const uint8_t pinFake, const uint8_t address, const uint16_t bufferSize = 30);

  bool available();
  void changeAddress(const uint8_t addr);

  void clearBuffer() {
    if (stream_->available()) {
      stream_->readBytes(buffer_, stream_->available());
      debugOutput("clearBuffer", true);
    }
    isData_ = false;
  }
  uint16_t getErrorCounter() const;
  uint8_t getAddress() const;
  uint8_t getRXaddress() const;
  uint8_t getTXaddress() const;
  static uint8_t getPacketSize() {
    return MIN_PACKET_SIZE_;
  }

  bool send(uint8_t address, char* data, uint16_t size, bool needCommiting = false);
  void sendBroadcast(char* data, uint16_t size);
  char* getData() {
    isData_ = false;
    return buffer_;
  }
  uint16_t getSizeData() {
    return sizeData_;
  }

  // private:

  void activateRecieveMode();
  void activateSendMode();

  static int8_t crc8(char* buffer, uint16_t size);
  bool parsePacket();
  void sendCommit();
  bool waitCommit();
  void debugOutput(const char* message, bool newLine = true);
};

SerialBusSlave::SerialBusSlave(const uint8_t pinRX, const uint8_t pinTX, const uint8_t pinFake, const uint8_t address, const uint16_t bufferSize)
  : pinRX_(pinRX), pinTX_(pinTX), stream2_(new SoftwareSerial(pinFake, pinTX)), stream_(new SoftwareSerial(pinRX, pinFake)), address_(address), bufferSize_(bufferSize) {

  if (bufferSize_ < 30)
    bufferSize_ = 30;

  buffer_ = new char[bufferSize_];

  stream_->begin(SERIAL_SPEED);

  stream2_->end();
  pinMode(pinTX_, INPUT_PULLUP);

  isRecieveMode_ = true;
}

bool SerialBusSlave::available() {

  if (isData_) {
    debugOutput("isData_ ERROR!", true);
    return true;
  }

  if (!stream_->available()) {
    // debugOutput("!stream_->available() ERROR!", true);
    return false;
  }

  while (stream_->available()) {

    if (parsePacket() == false) {
      stream_->readBytes(buffer_, stream_->available());  // clear stream_ buffer
      ++errorCounter_;
      break;
    }

    if (addrTo_ == BROADCAST_ADDR || addrTo_ == address_) {
      isData_ = true;
      debugOutput("DATA HIT!", true);
      break;
    }
  }

  return isData_;
}

void SerialBusSlave::changeAddress(const uint8_t addr) {
  address_ = addr;
}

uint16_t SerialBusSlave::getErrorCounter() const {
  return errorCounter_;
}

uint8_t SerialBusSlave::getAddress() const {
  return address_;
}

uint8_t SerialBusSlave::getRXaddress() const {
  return addrTo_;
}

uint8_t SerialBusSlave::getTXaddress() const {
  return addrFrom_;
}

bool SerialBusSlave::send(uint8_t addressTo, char* data, uint16_t size, bool needCommiting) {
  this->activateSendMode();

  stream_->write(START_SYMBOL);

  stream_->write(addressTo);
  stream_->write(address_);

  stream_->write((size >> 8) & 0xFF);
  stream_->write(size & 0x00FF);

  for (int i = 0; i < size; ++i) {
    stream_->write(data[i]);
  }

  stream_->write(crc8(data, size));
  debugOutput("CRC = ");
  debugOutput(String(crc8(data, size)).c_str(), true);

  int8_t isCommitInt8 = (needCommiting ? 1 : 0);
  stream_->write(isCommitInt8);

  this->activateRecieveMode();

  if (needCommiting) {

    return waitCommit();
  }

  return true;
}

void SerialBusSlave::sendBroadcast(char* data, uint16_t size) {
  send(BROADCAST_ADDR, data, size);
}

bool SerialBusSlave::parsePacket() {

  if (stream_->available() < MIN_PACKET_SIZE_) {
    delay(WAITING_DATA_TIMEOUT);
    if (stream_->available() < MIN_PACKET_SIZE_) {
      debugOutput("MIN_PACKET_SIZE_ ERROR!", true);
      return false;
    }
  }

  while (stream_->read() != START_SYMBOL) {
    if (stream_->available() < MIN_PACKET_SIZE_) {
      debugOutput("START_SYMBOL ERROR!", true);
      return false;
    }
  }

  addrTo_ = stream_->read();
  addrFrom_ = stream_->read();

  sizeData_ = stream_->read();
  sizeData_ = (sizeData_ << 8) | stream_->read();

  if (stream_->available() < sizeData_ + sizeof(int8_t) + sizeof(int8_t)) {  // sizeData_ + crcSize + iscommit byte
    delay(WAITING_DATA_TIMEOUT);
    if (stream_->available() < sizeData_ + sizeof(int8_t) + sizeof(int8_t)) {
      debugOutput("sizeData_ ERROR!", true);
      return false;
    }
  }

  stream_->readBytes(buffer_, sizeData_);

  int8_t crc = stream_->read();
  if (crc != crc8(buffer_, sizeData_)) {
    debugOutput("CRC ERROR! (crc -> ");
    debugOutput(String(crc, DEC).c_str());
    debugOutput(" != ");
    debugOutput(String(crc8(buffer_, sizeData_)).c_str());
    debugOutput(")", true);
    return false;
  }

  int8_t isCommit = stream_->read();

  if (isCommit == 1 && addrTo_ == address_) {
    sendCommit();
    debugOutput("commit is send");
  }

  return true;
}

void SerialBusSlave::activateRecieveMode() {
  if (isRecieveMode_) {
    return;
  }

  stream_->end();
  pinMode(pinTX_, INPUT_PULLUP);
  streamTemp_ = stream_;
  stream_ = stream2_;
  stream2_ = streamTemp_;
  stream_->begin(SERIAL_SPEED);
  isRecieveMode_ = true;

  debugOutput("activate Recieve Mode");
}

void SerialBusSlave::activateSendMode() {
  if (!isRecieveMode_) {
    return;
  }

  stream_->end();
  streamTemp_ = stream_;
  stream_ = stream2_;
  stream2_ = streamTemp_;
  stream_->begin(SERIAL_SPEED);
  isRecieveMode_ = false;

  debugOutput("activate Send Mode");
}

void SerialBusSlave::sendCommit() {
  this->activateSendMode();

  stream_->write(START_SYMBOL);

  stream_->write(addrFrom_);
  stream_->write(address_);

  stream_->write(uint8_t(0));  // :)  Compilation error: call of overloaded 'write(int)' is ambiguous
  stream_->write(1);

  stream_->write(1);  // any data with size == 1)

  stream_->write(48);  // crc for data

  stream_->write(uint8_t(0));  // commit byte == false (0)

  this->activateRecieveMode();
}

bool SerialBusSlave::waitCommit() {
  // save state

  bool isState = isData_;

  uint8_t addrToCopy = addrTo_;
  uint8_t addrFromCopy = addrFrom_;
  uint16_t sizeDataCopy = sizeData_;
  char* bufferCopy = nullptr;

  if (isState) {
    debugOutput("save state (waitCommit)");
    bufferCopy = new char[sizeData_];
    for (int i = 0; i < sizeDataCopy; ++i)
      bufferCopy[i] = buffer_[i];
  }

  isData_ = false;
  bool isError = false;
  auto timer = millis();

  while (stream_->available() < MIN_PACKET_SIZE_) {
    delay(1);
    if (millis() - timer > WAITING_SEND_COMMIT) {
      isError = true;
      break;
    }
  }

  if (!isError) {
    this->available();
  }

  uint16_t sizeDataResponse = sizeData_;

  // restore state

  isData_ = isState;

  if (isState) {
    debugOutput("restore state (waitCommit)");
    addrTo_ = addrToCopy;
    addrFrom_ = addrFromCopy;
    sizeData_ = sizeDataCopy;
    for (int i = 0; i < sizeDataCopy; ++i)
      buffer_[i] = bufferCopy[i];

    delete[] bufferCopy;
  }

  if (isError == false && sizeDataResponse == 1) {
    debugOutput("commit ~successful~ (waitCommit)");
    return true;
  }

  debugOutput("commit ERROR! (waitCommit). INFO -> ", false);
  debugOutput("Size response data - ", false);
  debugOutput(String(sizeDataResponse).c_str());
  debugOutput("timer flag - ", false);
  debugOutput(isError ? "Timer ERROR" : "Timer is OK");
  return false;
}

int8_t SerialBusSlave::crc8(char* buffer, uint16_t size) {
  int8_t crc = 0;

  for (uint16_t i = 0; i < size; i++) {
    char data = buffer[i];
    for (int8_t j = 8; j > 0; j--) {
      crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
      data >>= 1;
    }
  }
  return crc;
}

void SerialBusSlave::debugOutput(const char* message, bool newLine) {
#ifdef DEBUG_
  Serial.print(message);
  if (newLine) { Serial.println(); }
#endif
}

#endif