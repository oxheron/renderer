#include "camera.h"

void Camera::pitch(float angle)
{
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
    front = glm::vec3(rot * glm::vec4(front, 1.0f));
    up = glm::vec3(rot * glm::vec4(up, 1.0f));
}

void Camera::yaw(float angle)
{
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
    front = glm::vec3(rot * glm::vec4(front, 1.0f));
    up = glm::vec3(rot * glm::vec4(up, 1.0f));
}

void Camera::roll(float angle)
{
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
    front = glm::vec3(rot * glm::vec4(front, 1.0f));
    up = glm::vec3(rot * glm::vec4(up, 1.0f));
}

void Camera::movex(float amount)
{
    eye += glm::normalize(glm::cross(front, up)) * amount;
}

void Camera::movey(float amount)
{
    eye += up * amount;
}

void Camera::movez(float amount)
{
    eye += front * amount;
}