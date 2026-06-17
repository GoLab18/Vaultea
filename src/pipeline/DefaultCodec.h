#pragma once

#include "Codec.h"
#include "crypto/Constants.h"
#include "pipeline/ICompressor.h"

#include <memory>

using namespace vault::crypto;
using namespace vault::storage;

class DefaultCodec : public Codec {
public:
  DefaultCodec(Key masterKey);

  RawBytes encodeData(const RawBytes &plain) override;
  RawBytes decodeData(const RawBytes &encoded) override;

  RawBytes encodeIndex(const RawBytes &plain) override;
  RawBytes decodeIndex(const RawBytes &encoded) override;

  RawBytes encodeHeader(const RawBytes &plain) override;
  RawBytes decodeHeader(const RawBytes &encoded) override;

private:
  Key masterKey;

  std::unique_ptr<ICompressor> lz4;
  std::unique_ptr<ICompressor> zstd;
};
