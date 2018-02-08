#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

glm::mat4 Transform(const glm::vec3 &pos   = glm::vec3(), const glm::vec3 &rot = glm::vec3(),
                    const glm::vec3 &scale = glm::vec3(1.0f, 1.0f, 1.0f));
glm::vec3 Forward(float lookX, float lookZ);
glm::mat4 Camera(const glm::vec3 &pos, float lookX, float lookZ, float fov, float aspect, float zFar = 1000.0f, float zNear = 0.01f);
