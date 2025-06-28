#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gui
{
    class Shader
    {
    public:
        Shader(std::string vertexPath, std::string fragmentPath);
        Shader();
        void use();
        void destroy();
        void uniform1i(const char *name, int value);
        void uniform4mat(const char *name, glm::mat4 &value);

    private:
        std::string readFile(std::string path);
        unsigned int shaderProgram;
    };
}

#endif