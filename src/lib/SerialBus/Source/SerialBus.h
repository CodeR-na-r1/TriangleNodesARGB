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

  uint16_t errorCounter_ = 0;
  static constexpr uint8_t MIN_PACKET_SIZE_ = sizeof(address_) + sizeof(addrFrom_) + sizeof(sizeData_) + sizeof(char);

public:

  SerialBus(Stream* stream, uint8_t address, uint16_t bufferSize = 30);

  bool available();
  void changeAddress(const uint8_t addr);

  uint16_t getErrorCounter() const;
  uint8_t getTXaddress() const;
  static uint8_t getPacketSize() {
    return MIN_PACKET_SIZE_;
  }

  void send(uint8_t address, char* data, uint16_t size);
  void sendBroadcast(char* data, uint16_t size);
  char* getData() {
    isData_ = false;
    return buffer_;
  }
  void read();

private:

  bool parsePacket();
};

SerialBus::SerialBus(Stream* stream, uint8_t address, uint16_t bufferSize)
  : stream_(stream), address_(address), bufferSize_(bufferSize) {

  if (bufferSize_ < 30)
    bufferSize_ = 30;

  buffer_ = new char[bufferSize_];
}

bool SerialBus::available() {

  if (isData_) { return true; }

  if (!stream_->available()) { return false; }

  while (stream_->available()) {

    if (parsePacket() == false) {
      stream_->readBytes(buffer_, stream_->available());  // clear stream_ buffer
      ++errorCounter_;
      break;
    }

    if (addrTo_ == BROADCAST_ADDR || addrTo_ == address_) {
      isData_ = true;
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

void SerialBus::send(uint8_t addressTo, char* data, uint16_t size) {
  stream_->write(START_SYMBOL);

  stream_->write(addressTo);
  stream_->write(address_);

  stream_->write((size >> 8) & 0xFF);
  stream_->write(size & 0x00FF);

  for (int i = 0; i < size; ++i) {
    stream_->write(data[i]);
  }
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
      return false;
    }
  }

  while (stream_->read() != START_SYMBOL) {
    if (stream_->available() < MIN_PACKET_SIZE_) { return false; }
  }

  addrTo_ = stream_->read();
  addrFrom_ = stream_->read();

  sizeData_ = stream_->read();
  sizeData_ = (sizeData_ << 8) | stream_->read();

  if (stream_->available() < sizeData_) { return false; }

  stream_->readBytes(buffer_, sizeData_);

  return true;
}

#endif