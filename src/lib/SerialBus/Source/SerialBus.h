/*

  Структура пакета:
  TODO  

*/


#ifndef SERIAL_BUS
#define SERIAL_BUS

#define START_SYMBOL 3
#define WAITING_DATA_TIMEOUT 15

#define BROADCAST_ADDR 0

class SerialBus {

private:

  Stream* stream_;
  uint8_t address_;
  uint16_t bufferSize_;

  char* buffer_;

  bool isData_ = false;
  uint8_t addrTo_;
  uint8_t addrFrom_;
  uint16_t sizeData_;

  uint8_t maxRetries_ = 3;

  uint16_t errorCounter_ = 0;
  static constexpr uint8_t MIN_PACKET_SIZE_ = 1 + sizeof(address_) + sizeof(addrFrom_) + sizeof(sizeData_) + sizeof(char) + sizeof(int8_t) + sizeof(int8_t);

public:

  SerialBus(Stream* stream, uint8_t address, uint16_t bufferSize = 30);

  bool available();
  void changeAddress(const uint8_t addr);
  void setRetries(uint8_t retries) {
    maxRetries_ = retries;
  }

  uint16_t getErrorCounter() const;
  uint8_t getTXaddress() const;
  static uint8_t getPacketSize() {
    return MIN_PACKET_SIZE_;
  }

  void send(uint8_t address, char* data, uint16_t size, bool needCommiting = false);
  void sendBroadcast(char* data, uint16_t size);
  char* getData() {
    isData_ = false;
    return buffer_;
  }
  uint16_t getSizeData() {
    return sizeData_;
  }
  void read();

  // private:

  static int8_t crc8(char* buffer, uint16_t size);
  bool parsePacket();
  void debugOutput(const char* message, bool newLine = true);
};

SerialBus::SerialBus(Stream* stream, uint8_t address, uint16_t bufferSize)
  : stream_(stream), address_(address), bufferSize_(bufferSize) {

  if (bufferSize_ < 30)
    bufferSize_ = 30;

  buffer_ = new char[bufferSize_];
}

bool SerialBus::available() {

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

void SerialBus::changeAddress(const uint8_t addr) {
  address_ = addr;
}

uint16_t SerialBus::getErrorCounter() const {
  return errorCounter_;
}

uint8_t SerialBus::getTXaddress() const {
  return addrFrom_;
}

void SerialBus::send(uint8_t addressTo, char* data, uint16_t size, bool needCommiting) {
  stream_->write(START_SYMBOL);

  stream_->write(addressTo);
  stream_->write(address_);

  stream_->write((size >> 8) & 0xFF);
  stream_->write(size & 0x00FF);

  for (int i = 0; i < size; ++i) {
    stream_->write(data[i]);
  }

  stream_->write(crc8(data, size));
  Serial.print("CRC = ");
  Serial.println(crc8(data, size));

  Serial
}

void SerialBus::sendBroadcast(char* data, uint16_t size) {
  send(BROADCAST_ADDR, data, size);
}

void SerialBus::read() {
}

bool SerialBus::parsePacket() {

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

  if (stream_->available() < sizeData_ + sizeof(int8_t)) {  // sizeData_ + crcSize
    delay(WAITING_DATA_TIMEOUT);
    if (stream_->available() < sizeData_ + sizeof(int8_t)) {
      debugOutput("sizeData_ ERROR!", true);
      return false;
    }
  }

  stream_->readBytes(buffer_, sizeData_);

  int8_t crc = stream_->read();
  if (crc != crc8(buffer_, sizeData_)) {
    debugOutput("CRC ERROR! (crc -> ");
    debugOutput(crc);
    debugOutput(" != ");
    debugOutput(crc8(buffer_, sizeData_));
    debugOutput(")", true);
    return false;
  }

  return true;
}

int8_t SerialBus::crc8(char* buffer, uint16_t size) {
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

void SerialBus::debugOutput(const char* message, bool newLine) {
#ifdef DEBUG_
  Serial.print(message);
  if (newLine) { Serial.println(); }
#endif
}

#endif