#pragma once

// external
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// The basic camera
// Has methods for moving around, however, more than that isn't implemented because it could depend on the type of application being developed
class Camera
{
private:
    glm::vec3 eye = glm::vec3(0.0f, 0.0f, 0.0f);
    // The starting front position to rotated against
    glm::vec3 start_front = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float pitch, roll, yaw;

    glm::mat4 view;
public:
    Camera() {}
    Camera(const glm::vec3& eye, const glm::vec3& front, const glm::vec3& up) : eye(eye), front(front), up(up) {}
   
    void set_eye(const glm::vec3& eye) { this->eye = eye; }
    void update_front();
    void add_pitch(float angle);
    void add_yaw(float angle);
    void add_roll(float angle);
    void movex(float amount);
    void movey(float amount);
    void movez(float amount);

    inline glm::mat4& get_view() { view = glm::lookAt(eye, eye + front, up); return view; }
};
