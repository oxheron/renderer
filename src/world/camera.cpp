#include "camera.h"

void Camera::add_pitch(float angle)
{
    pitch += angle;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.f) pitch = 89.0f;
    update_front();
}

void Camera::add_yaw(float angle)
{
    yaw += angle;
}

void Camera::add_roll(float angle)
{
    roll += angle;
}

void Camera::update_front()
{
    auto rw = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0, 1, 0));
    auto rv = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1, 0, 0));
    auto ru = glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0, 0, 1));

    auto r = rw * rv * ru;
    front = glm::vec3(r * glm::vec4(start_front, 1.0f)); 
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
