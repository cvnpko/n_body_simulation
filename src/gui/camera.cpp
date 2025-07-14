#include "gui/camera.hpp"

namespace gui
{
    Camera::Camera(const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT)
        : cameraPos(glm::vec3(0.0f, 0.0f, 3.0f)), cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)), cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
          yaw(-90.0f), pitch(0.0f), lastX(SCR_WIDTH / 2.0f), lastY(SCR_HEIGHT / 2.0f), fov(45.0f),
          firstMouse(true), dontRotate(false), cameraSpeed(0.5f) {}
    float Camera::getFov() const
    {
        return fov;
    }
    glm::vec3 Camera::getCameraPos() const
    {
        return cameraPos;
    }
    glm::mat4 Camera::lookAt() const
    {
        return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }
    void Camera::updateKeyboard(int key, float deltaTime)
    {
        switch (key)
        {
        case GLFW_KEY_A:
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * deltaTime * cameraSpeed;
            break;
        case GLFW_KEY_W:
            cameraPos += cameraSpeed * deltaTime * cameraFront;
            break;
        case GLFW_KEY_S:
            cameraPos -= cameraSpeed * deltaTime * cameraFront;
            break;
        case GLFW_KEY_D:
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * deltaTime * cameraSpeed;
            break;
        }
    }
    void Camera::updateMouse(float xpos, float ypos)
    {
        if (!dontRotate)
        {
            if (firstMouse)
            {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
                return;
            }
            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos;
            lastX = xpos;
            lastY = ypos;

            float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw += xoffset;
            pitch += yoffset;

            pitch = glm::clamp(pitch, -89.0f, 89.0f);

            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraFront = glm::normalize(direction);
        }
    }
    void Camera::updateMouseClick(int button)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            dontRotate = false;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            firstMouse = true;
            dontRotate = true;
        }
    }
}