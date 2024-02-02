#include "camera.h"

Camera::Camera()
{
    pitch = roll = yaw = 0;    
}

Camera::Camera(const glm::vec3& eye, const glm::vec3& front, const glm::vec3& up) 
    : eye(eye), front(front), up(up), pitch(0), yaw(0), roll(0)
{}

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
    update_front();
}

void Camera::add_roll(float angle)
{
    roll += angle;
    update_front();
}

void Camera::update_front()
{
    auto ru = glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0, 0, 1));
    auto rw = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0, 1, 0));
    auto rv = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1, 0, 0));

    auto r = ru * rw * rv;
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
