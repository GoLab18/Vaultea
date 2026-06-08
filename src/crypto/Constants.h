#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <sodium.h>

namespace vault::crypto {

inline constexpr std::size_t KEY_SIZE =
    crypto_aead_chacha20poly1305_IETF_KEYBYTES;

inline constexpr std::size_t NONCE_SIZE =
    crypto_aead_chacha20poly1305_IETF_NPUBBYTES;

inline constexpr std::size_t SALT_SIZE = crypto_pwhash_SALTBYTES;

inline constexpr std::size_t MAC_SIZE =
    crypto_aead_chacha20poly1305_IETF_ABYTES;

inline constexpr std::size_t KEY_CHECK_SIZE = crypto_auth_hmacsha256_BYTES;

inline constexpr const char *KEY_CHECK_CONTEXT = "vault:keycheck:v1";

using Key = std::array<uint8_t, KEY_SIZE>;

using Nonce = std::array<uint8_t, NONCE_SIZE>;

using Salt = std::array<uint8_t, SALT_SIZE>;

using KeyCheck = std::array<uint8_t, KEY_CHECK_SIZE>;

} // namespace vault::crypto
