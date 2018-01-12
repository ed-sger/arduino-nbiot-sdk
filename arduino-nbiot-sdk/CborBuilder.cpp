#include "CborBuilder.h"
#include "Arduino.h"
#include <stdlib.h>

CborBuilder::CborBuilder(ATT_NBIOT &nbiot) {
  _nbiot = &nbiot;
  init(256);
}

CborBuilder::CborBuilder(ATT_NBIOT &nbiot, const uint32_t initalCapacity) {
  _nbiot = &nbiot;
  init(initalCapacity);
}

CborBuilder::~CborBuilder() {
  delete buffer;
}

void CborBuilder::init(unsigned int initalCapacity) {
  this->capacity = initalCapacity;
  this->buffer = new unsigned char[initalCapacity];
  this->offset = 0;
}

void CborBuilder::reset()
{
  //this->buffer = new unsigned char[capacity];
  this->offset = 0;
}

bool CborBuilder::send()
{
  return _nbiot->sendCbor(buffer, offset);
}

void CborBuilder::addInteger(String asset, int value)
{
  writeMap(1);
  writeString(asset);
  writeInt(value);
}

void CborBuilder::printCbor()
{
  return _nbiot->printCbor(buffer, offset);
}

unsigned char *CborBuilder::getData() {
  return buffer;
}

unsigned int CborBuilder::getSize() {
  return offset;
}

void CborBuilder::putByte(unsigned char value) {
  /*
  if(offset < capacity) {
    buffer[offset++] = value;
  } else {
    capacity *= 2;
    buffer = (unsigned char *) realloc(buffer, capacity);
    buffer[offset++] = value;
  }
  */
  
  if(offset < capacity) {
    buffer[offset++] = value;
  } else {
    Serial.print("buffer overflow error");
  }
}

void CborBuilder::putBytes(const unsigned char *data, const unsigned int size) {
  /*
  while(offset + size > capacity) {
    capacity *= 2;
    buffer = (unsigned char *) realloc(buffer, capacity);
  }
  memcpy(buffer + offset, data, size);
  offset += size;
  */
  
  if(offset + size - 1 < capacity) {
    memcpy(buffer + offset, data, size);
    offset += size;
  } else {
    Serial.print("buffer overflow error");
  }
}

// Writer functions

void CborBuilder::writeTypeAndValue(uint8_t majorType, const uint32_t value) {
  majorType <<= 5;
  if(value < 24) {
    putByte(majorType | value);
  } else if(value < 256) {
    putByte(majorType | 24);
    putByte(value);
  } else if(value < 65536) {
    putByte(majorType | 25);
    putByte(value >> 8);
    putByte(value);
  } else {
    putByte(majorType | 26);
    putByte(value >> 24);
    putByte(value >> 16);
    putByte(value >> 8);
    putByte(value);
  }
}

void CborBuilder::writeTypeAndValue(uint8_t majorType, const uint64_t value) {
  majorType <<= 5;
  if(value < 24ULL) {
    putByte(majorType | value);
  } else if(value < 256ULL) {
    putByte(majorType | 24);
    putByte(value);
  } else if(value < 65536ULL) {
    putByte(majorType | 25);
    putByte(value >> 8);
  } else if(value < 4294967296ULL) {
    putByte(majorType | 26);
    putByte(value >> 24);
    putByte(value >> 16);
    putByte(value >> 8);
    putByte(value);
  } else {
    putByte(majorType | 27);
    putByte(value >> 56);
    putByte(value >> 48);
    putByte(value >> 40);
    putByte(value >> 32);
    putByte(value >> 24);
    putByte(value >> 16);
    putByte(value >> 8);
    putByte(value);
  }
}

void CborBuilder::writeInt(const int value) {
  // This will break on 64-bit platforms
  writeTypeAndValue(0, (uint32_t)value);
}

void CborBuilder::writeInt(const uint32_t value) {
  writeTypeAndValue(0, value);
}

void CborBuilder::writeInt(const uint64_t value) {
  writeTypeAndValue(0, value);
}

void CborBuilder::writeInt(const int64_t value) {
  if(value < 0) {
    writeTypeAndValue(1, (uint64_t) -(value+1));
  } else {
    writeTypeAndValue(0, (uint64_t) value);
  }
}

void CborBuilder::writeInt(const int32_t value) {
  if(value < 0) {
    writeTypeAndValue(1, (uint32_t) -(value+1));
  } else {
    writeTypeAndValue(0, (uint32_t) value);
  }
}

void CborBuilder::writeBytes(const unsigned char *data, const unsigned int size) {
  writeTypeAndValue(2, (uint32_t)size);
  putBytes(data, size);
}

void CborBuilder::writeString(const char *data, const unsigned int size) {
  writeTypeAndValue(3, (uint32_t)size);
  putBytes((const unsigned char *)data, size);
}

void CborBuilder::writeString(const String str) {
  writeTypeAndValue(3, (uint32_t)str.length());
  putBytes((const unsigned char *)str.c_str(), str.length());
}

void CborBuilder::writeArray(const unsigned int size) {
  writeTypeAndValue(4, (uint32_t)size);
}

void CborBuilder::writeMap(const unsigned int size) {
  writeTypeAndValue(5, (uint32_t)size);
}

void CborBuilder::writeTag(const uint32_t tag) {
  writeTypeAndValue(6, tag);
}

void CborBuilder::writeSpecial(const uint32_t special) {
  writeTypeAndValue(7, special);
}