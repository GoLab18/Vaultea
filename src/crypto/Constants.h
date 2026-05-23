#pragma once

#include <cstddef>
#include <sodium.h>

namespace vault::crypto {

inline constexpr std::size_t KEY_SIZE =
    crypto_aead_chacha20poly1305_IETF_KEYBYTES;

inline constexpr std::size_t NONCE_SIZE =
    crypto_aead_chacha20poly1305_IETF_NPUBBYTES;

inline constexpr std::size_t SALT_SIZE = crypto_pwhash_SALTBYTES;

inline constexpr std::size_t MAC_SIZE =
    crypto_aead_chacha20poly1305_IETF_ABYTES;

} // namespace vault::crypto
