#pragma once

#include "Constants.h"

#include <ctime>

using namespace vault::storage;

inline EpochTime now() { return static_cast<EpochTime>(std::time(nullptr)); }
