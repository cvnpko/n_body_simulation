#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>

namespace gui
{
    class Shader
    {
    public:
        Shader(std::string vertexPath, std::string fragmentPath);
        Shader();
        void use();
        void destroy();

    private:
        std::string readFile(std::string path);
        unsigned int shaderProgram;
    };
}

#endif