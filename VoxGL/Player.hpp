#pragma once

#include <string>
#include <memory>

#include "glm/glm.hpp"

#include "Item.hpp"

struct Player {
  std::string name;

  glm::ivec3 iPos;
  std::array<std::unique_ptr<Item>, 4 * 9> inventory;
};
