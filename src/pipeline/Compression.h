#pragma once

#include "storage/Constants.h"

using namespace vault::storage;

class Compression {
public:
  static RawBytes compress(const RawBytes &input);
  static RawBytes decompress(const RawBytes &input);
};
