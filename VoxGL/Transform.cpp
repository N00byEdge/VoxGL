#include "Transform.hpp"

#include <iostream>

const glm::vec3 up{ 0, 0, 1 };
const float pid2 = acos(-1.0f) / 2;

glm::mat4 transform(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scale) {
	glm::mat4 posMatrix = glm::translate(pos);
	glm::mat4 scaleMatrix = glm::scale(scale);
	glm::mat4 rotMatrix = glm::rotate(rot.x, glm::vec3(1, 0, 0)) * glm::rotate(rot.y, glm::vec3(0, 1, 0)) * glm::rotate(rot.z, glm::vec3(0, 0, 1));

	return posMatrix * rotMatrix * scaleMatrix;
}

glm::vec3 forward(float lookX, float lookZ) {
	return glm::vec3(cos(lookX) * cos(lookZ), sin(lookX) * cos(lookZ), sin(lookZ));
}

glm::mat4 camera(const glm::vec3 &pos, float lookX, float lookZ, float fov, float aspect, float zFar, float zNear) {
	glm::vec3 fwd{ forward(lookX, lookZ) }, up{ forward(lookX, lookZ + pid2) };
	return glm::perspective(fov, aspect, zNear, zFar) * glm::lookAt(pos, pos + fwd, up);
}
