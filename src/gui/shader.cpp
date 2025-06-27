#include "gui/shader.hpp"

namespace gui
{
    Shader::Shader()
    {
        shaderProgram = 0;
    }
    Shader::Shader(std::string vertexPath, std::string fragmentPath)
    {
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        std::string vertexShaderSourceStr = readFile(vertexPath);
        const char *vertexShaderSource = vertexShaderSourceStr.c_str();
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        std::string fragmentShaderSourceStr = readFile("resources/shaders/fragmentShaders/threeBodies2d.frag");
        char const *fragmentShaderSource = fragmentShaderSourceStr.c_str();
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                      << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    std::string Shader::readFile(std::string path)
    {
        std::ifstream file;
        file.open(path);
        std::stringstream ret;
        ret << file.rdbuf();
        file.close();
        return ret.str();
    }
    void Shader::destroy()
    {
        glDeleteProgram(shaderProgram);
    }
    void Shader::use()
    {
        glUseProgram(shaderProgram);
    }
}