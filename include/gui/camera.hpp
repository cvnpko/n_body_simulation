#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

namespace gui
{
    class Camera
    {
    public:
        Camera(const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT);
        float getFov() const;
        glm::mat4 lookAt() const;
        glm::vec3 getCameraPos() const;
        void updateKeyboard(int button, float deltaTime);
        void updateFov(float offset);
        void updateMouse(float xpos, float ypos);
        void updateMouseClick(int button);

    private:
        glm::vec3 cameraPos, cameraFront, cameraUp;
        float yaw, pitch;
        float lastX, lastY;
        float fov;
        bool firstMouse, dontRotate;
        float cameraSpeed;
    };
}

#endif