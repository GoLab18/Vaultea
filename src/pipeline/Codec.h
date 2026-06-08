#pragma once

#include "storage/Constants.h"

using namespace vault::storage;

class Codec {
public:
  virtual ~Codec() = default;

  virtual RawBytes encodeData(const RawBytes &plain) = 0;
  virtual RawBytes decodeData(const RawBytes &enocded) = 0;

  virtual RawBytes encodeIndex(const RawBytes &plain) = 0;
  virtual RawBytes decodeIndex(const RawBytes &enocded) = 0;

  virtual RawBytes encodeHeader(const RawBytes &plain) = 0;
  virtual RawBytes decodeHeader(const RawBytes &encoded) = 0;
};
