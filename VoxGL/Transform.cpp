#include "Transform.hpp"

#include <iostream>

const glm::vec3 Up{0, 0, 1};
const float Pid2 = acos(-1.0f) / 2;

glm::mat4 Transform(glm::vec3 const &pos, glm::vec3 const &rot, glm::vec3 const &scale) {
  auto const posMatrix   = translate(pos);
  auto const scaleMatrix = glm::scale(scale);
  auto const rotMatrix   = rotate(rot.x, glm::vec3(1, 0, 0)) * rotate(rot.y, glm::vec3(0, 1, 0)) * rotate(rot.z, glm::vec3(0, 0, 1));

  return posMatrix * rotMatrix * scaleMatrix;
}

glm::vec3 Forward(float const lookX, float const lookZ) { return glm::vec3(cos(lookX) * cos(lookZ), sin(lookX) * cos(lookZ), sin(lookZ)); }

glm::mat4 Camera(const glm::vec3 &pos, float const lookX, float const lookZ, float const fov, float const aspect, float const zFar,
                 float const zNear) {
  auto const fwd{Forward(lookX, lookZ)};
  auto const up{Forward(lookX, lookZ + Pid2)};
  return glm::perspective(fov, aspect, zNear, zFar) * lookAt(pos, pos + fwd, up);
}
