#pragma once

#include "Codec.h"
#include "crypto/Constants.h"

using namespace vault::crypto;
using namespace vault::storage;

class DefaultCodec : public Codec {
public:
  DefaultCodec(Key masterKey);

  RawBytes encodeData(const RawBytes &plain) override;

  RawBytes decodeData(const RawBytes &encoded) override;

  RawBytes encodeIndex(const RawBytes &plain) override;

  RawBytes decodeIndex(const RawBytes &enocded) override;

private:
  Key masterKey;
};
