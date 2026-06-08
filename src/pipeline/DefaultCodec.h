#pragma once

#include "Codec.h"
#include "crypto/Constants.h"
#include "pipeline/ICompressor.h"

#include <memory>

using namespace vault::crypto;
using namespace vault::storage;

class DefaultCodec : public Codec {
public:
  DefaultCodec(Key masterKey, std::shared_ptr<ICompressor> compressor);

  RawBytes encodeData(const RawBytes &plain) override;
  RawBytes decodeData(const RawBytes &encoded) override;

  RawBytes encodeIndex(const RawBytes &plain) override;
  RawBytes decodeIndex(const RawBytes &encoded) override;

  RawBytes encodeHeader(const RawBytes &plain) override;
  RawBytes decodeHeader(const RawBytes &encoded) override;

private:
  Key masterKey;
  std::shared_ptr<ICompressor> compressor;
};
