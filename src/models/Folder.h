#pragma once

#include "BaseVaultObject.h"

struct Folder : public BaseVaultObject {
  std::string name;
  std::string description;
};
