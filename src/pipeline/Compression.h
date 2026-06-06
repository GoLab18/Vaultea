#pragma once

#include "storage/Constants.h"

#include <cstdint>
#include <vector>

using namespace vault::storage;

class Compression {
public:
  static RawBytes compress(const RawBytes &input);
  static RawBytes decompress(const RawBytes &input);
};
