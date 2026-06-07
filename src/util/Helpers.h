#pragma once

#include <algorithm>
#include <cstdint>
#include <ctime>
#include <string>

namespace vault::util {

inline std::string nextPrefix(const std::string &prefix) {
  std::string next = prefix;

  for (int i = static_cast<int>(next.size()) - 1; i >= 0; --i) {
    if (static_cast<unsigned char>(next[i]) != 255) {
      ++next[i];
      next.resize(i + 1);
      return next;
    }
  }

  return {};
}

// TODO handle full unicode normalization (for now only ASCII)
inline std::string normalize(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });

  return s;
}

namespace time {

using EpochTime = uint64_t;

inline EpochTime now() { return static_cast<EpochTime>(std::time(nullptr)); }

} // namespace time

} // namespace vault::util
