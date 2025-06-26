#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "simulation/body.hpp"
#include "simulation/simulation.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
std::string readFile(std::string path);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
sim::States state = sim::States::MENU;
std::vector<sim::Body2d> bodies2d;
std::vector<sim::Body3d> bodies3d;
std::vector<float> vertices;
unsigned int shaderProgram = 0;
unsigned int VBO = 0, VAO = 0;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "N Body Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create glfw window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize glad\n";
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiStyle &style = ImGui::GetStyle();
    style.FrameRounding = 20.0f;
    ImGui::GetIO().FontGlobalScale = 2.0f;
    glEnable(GL_PROGRAM_POINT_SIZE);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        switch (state)
        {
        case sim::States::MENU:
        {
            ImGuiIO &io = ImGui::GetIO();
            ImVec2 window_size = ImVec2(io.DisplaySize.x / 2.0f, io.DisplaySize.y);
            ImVec2 window_pos = ImVec2((io.DisplaySize.x - window_size.x) * 0.5f, 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("Controls", nullptr,
                         ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoBackground);
            ImVec2 button_size = ImVec2(window_size.x, window_size.y / 6.0f);
            ImVec2 dummy_size = ImVec2(window_size.x, window_size.y / 6.0f / 12.0f);
            ImGui::Dummy(dummy_size);
            if (ImGui::Button("Three bodies", button_size))
            {
                state = sim::States::ThreeBody2DInit;
            }
            ImGui::Dummy(dummy_size);
            if (ImGui::Button("Fixed 2 bodies", button_size))
            {
                state = sim::States::TwoFixedBodyInit;
            }
            ImGui::Dummy(dummy_size);
            if (ImGui::Button("Small n bodies", button_size))
            {
                state = sim::States::NBodySmallInit;
            }
            ImGui::Dummy(dummy_size);
            if (ImGui::Button("Big n bodies", button_size))
            {
                state = sim::States::NBodyBigInit;
            }
            ImGui::Dummy(dummy_size);
            if (ImGui::Button("Three bodies 3D", button_size))
            {
                state = sim::States::ThreeBody3DInit;
            }
            ImGui::End();
        }
        break;
        case sim::States::ThreeBody2DInit:
        {
            ImGuiIO &io = ImGui::GetIO();
            ImVec2 window_size = ImVec2(io.DisplaySize.x, io.DisplaySize.y);
            ImVec2 window_pos = ImVec2(0.0f, 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("Controls", nullptr,
                         ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoBackground);
            static float mass1 = 0.1f, mass2 = 0.1f, mass3 = 0.1f;
            static float x1 = 200.0f, x2 = -300.0, x3 = 100.0f, y1 = -500.0f, y2 = 50.0f, y3 = 0.0f;
            static float vx1 = 0.0f, vx2 = 0.0f, vx3 = 0.0f, vy1 = 0.0f, vy2 = 0.0f, vy3 = 0.0f;
            ImGui::BeginGroup();
            ImVec2 button_size = ImVec2(140, 60);
            if (ImGui::Button("Start", button_size))
            {
                bodies2d.clear();
                bodies2d.push_back(sim::Body2d(mass1, x1, y1, vx1, vy1));
                bodies2d.push_back(sim::Body2d(mass2, x2, y2, vx2, vy2));
                bodies2d.push_back(sim::Body2d(mass3, x3, y3, vx3, vy3));
                vertices.clear();
                vertices.push_back(x1 / 1000.0f);
                vertices.push_back(y1 / 1000.0f);
                vertices.push_back(x2 / 1000.0f);
                vertices.push_back(y2 / 1000.0f);
                vertices.push_back(x3 / 1000.0f);
                vertices.push_back(y3 / 1000.0f);

                if (shaderProgram != 0)
                {
                    glDeleteShader(shaderProgram);
                }
                unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
                std::string vertexShaderSourceStr = readFile("resources/shaders/vertexShaders/threeBodies2d.ver");
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

                if (shaderProgram != 0)
                {

                    glUseProgram(0);
                    glDeleteShader(shaderProgram);
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
                    return 1;
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
                    return 1;
                }
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);

                if (VAO != 0)
                {
                    glBindVertexArray(0);
                    glDeleteVertexArrays(1, &VAO);
                }
                if (VBO != 0)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glDeleteBuffers(1, &VBO);
                }
                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
                state = sim::States::ThreeBody2DSim;
            }
            static bool trail = false;
            ImGui::Checkbox("Trail", &trail);

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Dummy(ImVec2(100.0f, 0.0f));
            ImGui::SameLine();
            ImGui::BeginGroup();
            if (ImGui::InputFloat("mass1", &mass1, 0.1f, 1.0f, "%.2f"))
            {
                if (mass1 < -1000.0)
                {
                    mass1 = 0.1;
                }
                if (mass1 > 1000.0)
                {
                    mass1 = 1000.0;
                }
            }
            if (ImGui::InputFloat("x1", &x1, 0.1f, 1.0f, "%.2f"))
            {
                if (x1 < -1000.0)
                {
                    x1 = -1000.0;
                }
                if (x1 > 1000.0)
                {
                    x1 = 1000.0;
                }
            }
            if (ImGui::InputFloat("y1", &y1, 0.1f, 1.0f, "%.2f"))
            {
                if (y1 < -1000.0)
                {
                    y1 = -1000.0;
                }
                if (y1 > 1000.0)
                {
                    y1 = 1000.0;
                }
            }
            if (ImGui::InputFloat("vx1", &vx1, 0.1f, 1.0f, "%.2f"))
            {
                if (vx1 < -1000.0)
                {
                    vx1 = -1000.0;
                }
                if (vx1 > 1000.0)
                {
                    vx1 = 1000.0;
                }
            }
            if (ImGui::InputFloat("vy1", &vy1, 0.1f, 1.0f, "%.2f"))
            {
                if (vy1 < -1000.0)
                {
                    vy1 = -1000.0;
                }
                if (vy1 > 1000.0)
                {
                    vy1 = 1000.0;
                }
            }
            if (ImGui::InputFloat("mass2", &mass2, 0.1f, 1.0f, "%.2f"))
            {
                if (mass2 < -1000.0)
                {
                    mass2 = 0.1;
                }
                if (mass2 > 1000.0)
                {
                    mass2 = 1000.0;
                }
            }
            if (ImGui::InputFloat("x2", &x2, 0.1f, 1.0f, "%.2f"))
            {
                if (x2 < -1000.0)
                {
                    x2 = -1000.0;
                }
                if (x2 > 1000.0)
                {
                    x2 = 1000.0;
                }
            }
            if (ImGui::InputFloat("y2", &y2, 0.1f, 1.0f, "%.2f"))
            {
                if (y2 < -1000.0)
                {
                    y2 = -1000.0;
                }
                if (y2 > 1000.0)
                {
                    y2 = 1000.0;
                }
            }
            if (ImGui::InputFloat("vx2", &vx2, 0.1f, 1.0f, "%.2f"))
            {
                if (vx2 < -1000.0)
                {
                    vx2 = -1000.0;
                }
                if (vx2 > 1000.0)
                {
                    vx2 = 1000.0;
                }
            }
            if (ImGui::InputFloat("vy2", &vy2, 0.1f, 1.0f, "%.2f"))
            {
                if (vy2 < -1000.0)
                {
                    vy2 = -1000.0;
                }
                if (vy2 > 1000.0)
                {
                    vy2 = 1000.0;
                }
            }
            if (ImGui::InputFloat("mass3", &mass3, 0.1f, 1.0f, "%.2f"))
            {
                if (mass3 < -1000.0)
                {
                    mass3 = 0.1;
                }
                if (mass3 > 1000.0)
                {
                    mass3 = 1000.0;
                }
            }
            if (ImGui::InputFloat("x3", &x3, 0.1f, 1.0f, "%.2f"))
            {
                if (x3 < -1000.0)
                {
                    x3 = -1000.0;
                }
                if (x3 > 1000.0)
                {
                    x3 = 1000.0;
                }
            }
            if (ImGui::InputFloat("y3", &y3, 0.1f, 1.0f, "%.2f"))
            {
                if (y3 < -1000.0)
                {
                    y3 = -1000.0;
                }
                if (y3 > 1000.0)
                {
                    y3 = 1000.0;
                }
            }
            if (ImGui::InputFloat("vx3", &vx3, 0.1f, 1.0f, "%.2f"))
            {
                if (vx3 < -1000.0)
                {
                    vx3 = -1000.0;
                }
                if (vx3 > 1000.0)
                {
                    vx3 = 1000.0;
                }
            }
            if (ImGui::InputFloat("vy3", &vy3, 0.1f, 1.0f, "%.2f"))
            {
                if (vy3 < -1000.0)
                {
                    vy3 = -1000.0;
                }
                if (vy3 > 1000.0)
                {
                    vy3 = 1000.0;
                }
            }
            ImGui::EndGroup();
            ImGui::EndGroup();
            ImGui::End();
        }
        break;
        case sim::States::ThreeBody2DSim:
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, 3);
            break;
        case sim::States::TwoFixedBodyInit:
            break;
        case sim::States::TwoFixedBodySim:
            break;
        case sim::States::NBodySmallInit:
            break;
        case sim::States::NBodySmallSim:
            break;
        case sim::States::NBodyBigInit:
            break;
        case sim::States::NBodyBigSim:
            break;
        case sim::States::ThreeBody3DInit:
            break;
        case sim::States::ThreeBody3DSim:
            break;
        default:
            glfwSetWindowShouldClose(window, true);
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        switch (state)
        {
        case sim::States::NBodyBigSim:
            state = sim::States::NBodyBigInit;
            break;
        case sim::States::NBodySmallSim:
            state = sim::States::NBodySmallInit;
            break;
        case sim::States::TwoFixedBodySim:
            state = sim::States::TwoFixedBodyInit;
            break;
        case sim::States::ThreeBody2DSim:
            state = sim::States::ThreeBody2DInit;
            break;
        case sim::States::ThreeBody3DSim:
            state = sim::States::ThreeBody3DInit;
            break;
        default:
            state = sim::States::MENU;
        }
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

std::string readFile(std::string path)
{
    std::ifstream file;
    file.open(path);
    std::stringstream ret;
    ret << file.rdbuf();
    file.close();
    return ret.str();
}
