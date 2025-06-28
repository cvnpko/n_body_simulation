#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <random>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "simulation/quadTree.hpp"
#include "simulation/body.hpp"
#include "simulation/simulation.hpp"
#include "gui/shader.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void update2dBodies(std::vector<sim::Body2d> &bodies);

struct trailStruct
{
    float x, y;
    int index;
};

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
sim::States state = sim::States::MENU;
std::vector<sim::Body2d> bodies2d;
std::vector<sim::Body3d> bodies3d;
std::vector<float> vertices;
std::vector<trailStruct> trailVertices;
gui::Shader shaderProgram, shaderProgramTrail;
unsigned int VBO = 0, VAO = 0, trailVBO = 0, trailVAO = 0;
double currentTime;
const double G = 6700.0;
const double alpha = 5.0;
bool trail = false;
unsigned int trailLength = 500;
int numOfBodies = 0;
float theta = 2.0f;

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
    glfwSetKeyCallback(window, key_callback);

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
            std::vector<const char *> buttonNames({"ThreeBodies", "Fixed 2 bodies", "Small n bodies", "Big n bodies", "Three bodies 3D"});
            std::vector<sim::States> buttonStates({sim::States::ThreeBody2DInit, sim::States::TwoFixedBodyInit, sim::States::NBodySmallInit, sim::States::NBodyBigInit, sim::States::ThreeBody3DInit});
            ImVec2 button_size = ImVec2(window_size.x, window_size.y / 6.0f);
            ImVec2 dummy_size = ImVec2(window_size.x, window_size.y / 6.0f / 12.0f);
            for (int i = 0; i < buttonNames.size(); i++)
            {
                ImGui::Dummy(dummy_size);
                if (ImGui::Button(buttonNames[i], button_size))
                {
                    state = buttonStates[i];
                    switch (state)
                    {
                    case sim::States::ThreeBody2DInit:
                        numOfBodies = 3;
                        bodies2d = std::vector<sim::Body2d>(numOfBodies);
                        break;
                    case sim::States::TwoFixedBodyInit:
                        numOfBodies = 3;
                        bodies2d = std::vector<sim::Body2d>(numOfBodies);
                        break;
                    case sim::States::NBodySmallInit:
                        numOfBodies = 1;
                        bodies2d = std::vector<sim::Body2d>(numOfBodies);
                        break;
                    case sim::States::NBodyBigInit:
                        numOfBodies = 10000;
                        bodies2d = std::vector<sim::Body2d>(numOfBodies);
                    case sim::States::ThreeBody3DInit:
                        numOfBodies = 3;
                        bodies3d = std::vector<sim::Body3d>(numOfBodies);
                    }
                }
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
            ImGui::BeginGroup();
            ImVec2 button_size = ImVec2(140, 60);
            if (ImGui::Button("Start", button_size))
            {
                vertices.clear();
                vertices = std::vector<float>(3 * 2);
                for (int i = 0; i < 3; i++)
                {
                    vertices[i * 2] = bodies2d[i].x / 1000.0f;
                    vertices[i * 2 + 1] = bodies2d[i].y / 1000.0f;
                }
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver", "resources/shaders/fragmentShaders/threeBodies2d.frag");

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
                if (trail)
                {
                    trailVertices.clear();
                    trailVertices = std::vector<trailStruct>(trailLength * 3);
                    for (int i = 0; i < 3; i++)
                    {
                        for (int j = 0; j < trailLength; j++)
                        {
                            trailVertices[i * trailLength + j].x = vertices[i * 2];
                            trailVertices[i * trailLength + j].y = vertices[i * 2 + 1];
                            trailVertices[i * trailLength + j].index = j;
                        }
                    }
                    shaderProgramTrail = gui::Shader("resources/shaders/vertexShaders/threeBodies2dTrail.ver", "resources/shaders/fragmentShaders/threeBodies2dTrail.frag");

                    if (trailVAO != 0)
                    {
                        glBindVertexArray(0);
                        glDeleteVertexArrays(1, &trailVAO);
                    }
                    if (trailVBO != 0)
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glDeleteBuffers(1, &trailVBO);
                    }
                    glGenVertexArrays(1, &trailVAO);
                    glGenBuffers(1, &trailVBO);
                    glBindVertexArray(trailVAO);
                    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
                    glBufferData(GL_ARRAY_BUFFER, trailVertices.size() * sizeof(trailStruct), &trailVertices[0], GL_DYNAMIC_DRAW);
                    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(trailStruct), (void *)0);
                    glVertexAttribIPointer(1, 1, GL_INT, sizeof(trailStruct), (void *)(sizeof(float) * 2));
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glBindVertexArray(0);
                }
                currentTime = glfwGetTime();
                state = sim::States::ThreeBody2DSim;
            }
            ImGui::Checkbox("Trail", &trail);

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Dummy(ImVec2(100.0f, 0.0f));
            ImGui::SameLine();
            ImGui::BeginGroup();
            std::vector<const char *> inputFloatNames({"mass1", "x1", "y1", "vx1", "vy1", "mass2", "x2", "y2", "vx2", "vy2", "mass3", "x3", "y3", "vx3", "vy3"});
            for (int i = 0, j = 0; i < 3; i++)
            {
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies2d[i].mass, 0.1f, 1.0f, "%.2f"))
                {
                    bodies2d[i].mass = std::max(0.1f, std::min(bodies2d[i].mass, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies2d[i].x, 0.1f, 1.0f, "%.2f"))
                {
                    bodies2d[i].x = std::max(-1000.0f, std::min(bodies2d[i].x, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies2d[i].y, 0.1f, 1.0f, "%.2f"))
                {
                    bodies2d[i].y = std::max(-1000.0f, std::min(bodies2d[i].y, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies2d[i].vx, 0.1f, 1.0f, "%.2f"))
                {
                    bodies2d[i].vx = std::max(-1000.0f, std::min(bodies2d[i].vx, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies2d[i].vy, 0.1f, 1.0f, "%.2f"))
                {
                    bodies2d[i].vy = std::max(-1000.0f, std::min(bodies2d[i].vy, 1000.0f));
                }
            }
            ImGui::EndGroup();
            ImGui::EndGroup();
            ImGui::End();
        }
        break;
        case sim::States::ThreeBody2DSim:
        {
            if (trail)
            {
                shaderProgramTrail.use();
                shaderProgramTrail.uniform1i("uMaxIndex", trailLength);
                glBindVertexArray(trailVAO);
                glDrawArrays(GL_POINTS, 0, trailVertices.size());
            }
            shaderProgram.use();
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, 3);

            update2dBodies(bodies2d);
            if (trail)
            {
                for (int i = 0; i < bodies2d.size(); i++)
                {
                    for (int j = 0; j < trailLength - 1; j++)
                    {
                        trailVertices[i * trailLength + j].x = trailVertices[i * trailLength + j + 1].x;
                        trailVertices[i * trailLength + j].y = trailVertices[i * trailLength + j + 1].y;
                    }
                    trailVertices[(i + 1) * trailLength - 1].x = vertices[i * 2];
                    trailVertices[(i + 1) * trailLength - 1].y = vertices[i * 2 + 1];
                }
            }
            for (int i = 0; i < bodies2d.size(); i++)
            {
                vertices[i * 2] = bodies2d[i].x / 1000.0f;
                vertices[i * 2 + 1] = bodies2d[i].y / 1000.0f;
            }

            if (trail)
            {
                glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
                void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, trailVertices.size() * sizeof(trailStruct), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                if (ptr != NULL)
                {
                    memcpy(ptr, trailVertices.data(), trailVertices.size() * sizeof(trailStruct));
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                }
            }
            {
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                if (ptr != NULL)
                {
                    memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                }
            }
            currentTime = glfwGetTime();
        }
        break;
        case sim::States::TwoFixedBodyInit:
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
            ImGui::BeginGroup();
            ImVec2 button_size = ImVec2(140, 60);
            if (ImGui::Button("Start", button_size))
            {
                vertices.clear();
                vertices = std::vector<float>(3 * 2);
                for (int i = 0; i < 3; i++)
                {
                    vertices[i * 2] = bodies2d[i].x / 1000.0f;
                    vertices[i * 2 + 1] = bodies2d[i].y / 1000.0f;
                }
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver", "resources/shaders/fragmentShaders/threeBodies2d.frag");

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
                if (trail)
                {
                    trailVertices.clear();
                    trailVertices = std::vector<trailStruct>(trailLength);
                    for (int j = 0; j < trailLength; j++)
                    {
                        trailVertices[j].x = vertices[2];
                        trailVertices[j].y = vertices[2 + 1];
                        trailVertices[j].index = j;
                    }
                    shaderProgramTrail = gui::Shader("resources/shaders/vertexShaders/threeBodies2dTrail.ver", "resources/shaders/fragmentShaders/threeBodies2dTrail.frag");

                    if (trailVAO != 0)
                    {
                        glBindVertexArray(0);
                        glDeleteVertexArrays(1, &trailVAO);
                    }
                    if (trailVBO != 0)
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glDeleteBuffers(1, &trailVBO);
                    }
                    glGenVertexArrays(1, &trailVAO);
                    glGenBuffers(1, &trailVBO);
                    glBindVertexArray(trailVAO);
                    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
                    glBufferData(GL_ARRAY_BUFFER, trailVertices.size() * sizeof(trailStruct), &trailVertices[0], GL_DYNAMIC_DRAW);
                    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(trailStruct), (void *)0);
                    glVertexAttribIPointer(1, 1, GL_INT, sizeof(trailStruct), (void *)(sizeof(float) * 2));
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glBindVertexArray(0);
                }
                currentTime = glfwGetTime();
                state = sim::States::TwoFixedBodySim;
            }
            ImGui::Checkbox("Trail", &trail);

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Dummy(ImVec2(100.0f, 0.0f));
            ImGui::SameLine();
            ImGui::BeginGroup();
            if (ImGui::InputFloat("mass1", &bodies2d[0].mass, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[0].mass = std::max(0.1f, std::min(bodies2d[0].mass, 1000.0f));
            }
            if (ImGui::InputFloat("x1", &bodies2d[0].x, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[0].x = std::max(-1000.0f, std::min(bodies2d[0].x, 1000.0f));
            }
            if (ImGui::InputFloat("y1", &bodies2d[0].y, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[0].y = std::max(-1000.0f, std::min(bodies2d[0].y, 1000.0f));
            }
            if (ImGui::InputFloat("vx1", &bodies2d[0].vx, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[0].vx = std::max(-1000.0f, std::min(bodies2d[0].vx, 1000.0f));
            }
            if (ImGui::InputFloat("vy1", &bodies2d[0].vy, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[0].vy = std::max(-1000.0f, std::min(bodies2d[0].vy, 1000.0f));
            }
            std::vector<const char *> inputFloatNames({"mass2", "x2", "y2", "mass3", "x3", "y3"});
            for (int i = 1, j = 0; i < 3; i++)
            {
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies2d[i].mass, 0.1f, 1.0f, "%.2f"))
                {
                    bodies2d[i].mass = std::max(0.1f, std::min(bodies2d[i].mass, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies2d[i].x, 0.1f, 1.0f, "%.2f"))
                {
                    bodies2d[i].x = std::max(-1000.0f, std::min(bodies2d[i].x, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies2d[i].y, 0.1f, 1.0f, "%.2f"))
                {
                    bodies2d[i].y = std::max(-1000.0f, std::min(bodies2d[i].y, 1000.0f));
                }
            }
            ImGui::EndGroup();
            ImGui::EndGroup();
            ImGui::End();
        }
        break;
        case sim::States::TwoFixedBodySim:
        {
            if (trail)
            {
                shaderProgramTrail.use();
                shaderProgramTrail.uniform1i("uMaxIndex", trailLength);
                glBindVertexArray(trailVAO);
                glDrawArrays(GL_POINTS, 0, trailVertices.size());
            }
            shaderProgram.use();
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, 3);

            double newTime = glfwGetTime();
            double a[2] = {0, 0};
            for (int j = 1; j < bodies2d.size(); j++)
            {
                double dx = bodies2d[j].x - bodies2d[0].x;
                double dy = bodies2d[j].y - bodies2d[0].y;
                double distSqr = dx * dx + dy * dy + alpha * alpha;
                double invDist = 1.0 / sqrt(distSqr);
                double invDist3 = invDist * invDist * invDist;

                a[0] += G * bodies2d[j].mass * dx * invDist3;
                a[1] += G * bodies2d[j].mass * dy * invDist3;
            }

            bodies2d[0].vx += a[0] * (newTime - currentTime) * 10;
            bodies2d[0].vy += a[1] * (newTime - currentTime) * 10;
            bodies2d[0].x += bodies2d[0].vx * (newTime - currentTime);
            bodies2d[0].y += bodies2d[0].vy * (newTime - currentTime);

            if (trail)
            {
                for (int j = 0; j < trailLength - 1; j++)
                {
                    trailVertices[j].x = trailVertices[j + 1].x;
                    trailVertices[j].y = trailVertices[j + 1].y;
                }
                trailVertices[trailLength - 1].x = vertices[0];
                trailVertices[trailLength - 1].y = vertices[1];
            }

            vertices[0] = bodies2d[0].x / 1000.0f;
            vertices[1] = bodies2d[0].y / 1000.0f;

            if (trail)
            {
                glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
                void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, trailVertices.size() * sizeof(trailStruct), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                if (ptr != NULL)
                {
                    memcpy(ptr, trailVertices.data(), trailVertices.size() * sizeof(trailStruct));
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                }
            }
            {
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                if (ptr != NULL)
                {
                    memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                }
            }
            currentTime = glfwGetTime();
        }
        break;
        case sim::States::NBodySmallInit:
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
            ImVec2 button_size = ImVec2(140, 60);
            if (ImGui::Button("Start", button_size))
            {
                vertices.clear();
                vertices = std::vector<float>(numOfBodies * 2);
                for (int i = 0; i < numOfBodies; i++)
                {
                    vertices[i * 2] = bodies2d[i].x / 1000.0f;
                    vertices[i * 2 + 1] = bodies2d[i].y / 1000.0f;
                }
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver", "resources/shaders/fragmentShaders/threeBodies2d.frag");

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
                if (trail)
                {
                    trailVertices.clear();
                    trailVertices = std::vector<trailStruct>(trailLength * numOfBodies);
                    for (int i = 0; i < numOfBodies; i++)
                    {
                        for (int j = 0; j < trailLength; j++)
                        {
                            trailVertices[i * trailLength + j].x = vertices[i * 2];
                            trailVertices[i * trailLength + j].y = vertices[i * 2 + 1];
                            trailVertices[i * trailLength + j].index = j;
                        }
                    }
                    shaderProgramTrail = gui::Shader("resources/shaders/vertexShaders/threeBodies2dTrail.ver", "resources/shaders/fragmentShaders/threeBodies2dTrail.frag");

                    if (trailVAO != 0)
                    {
                        glBindVertexArray(0);
                        glDeleteVertexArrays(1, &trailVAO);
                    }
                    if (trailVBO != 0)
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glDeleteBuffers(1, &trailVBO);
                    }
                    glGenVertexArrays(1, &trailVAO);
                    glGenBuffers(1, &trailVBO);
                    glBindVertexArray(trailVAO);
                    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
                    glBufferData(GL_ARRAY_BUFFER, trailVertices.size() * sizeof(trailStruct), &trailVertices[0], GL_DYNAMIC_DRAW);
                    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(trailStruct), (void *)0);
                    glVertexAttribIPointer(1, 1, GL_INT, sizeof(trailStruct), (void *)(sizeof(float) * 2));
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glBindVertexArray(0);
                }
                currentTime = glfwGetTime();
                state = sim::States::NBodySmallSim;
            }
            if (ImGui::InputInt("Number of bodies", &numOfBodies, 1, 3))
            {
                numOfBodies = std::min(10, std::max(numOfBodies, 1));
                while (bodies2d.size() < numOfBodies)
                {
                    bodies2d.push_back(sim::Body2d());
                }
                while (bodies2d.size() > numOfBodies)
                {
                    bodies2d.pop_back();
                }
            }
            static int selectedBody = 0;
            if (ImGui::InputInt("Selected body", &selectedBody, 1, 3))
            {
                selectedBody = std::min(numOfBodies - 1, std::max(selectedBody, 0));
            }
            ImGui::Checkbox("Trail", &trail);
            if (ImGui::InputFloat("mass", &bodies2d[selectedBody].mass, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[selectedBody].mass = std::max(0.1f, std::min(bodies2d[selectedBody].mass, 1000.0f));
            }
            if (ImGui::InputFloat("x", &bodies2d[selectedBody].x, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[selectedBody].x = std::max(-1000.0f, std::min(bodies2d[selectedBody].x, 1000.0f));
            }
            if (ImGui::InputFloat("y", &bodies2d[selectedBody].y, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[selectedBody].y = std::max(-1000.0f, std::min(bodies2d[selectedBody].y, 1000.0f));
            }
            if (ImGui::InputFloat("vx", &bodies2d[selectedBody].vx, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[selectedBody].vx = std::max(-1000.0f, std::min(bodies2d[selectedBody].vx, 1000.0f));
            }
            if (ImGui::InputFloat("vy", &bodies2d[selectedBody].vy, 0.1f, 1.0f, "%.2f"))
            {
                bodies2d[selectedBody].vy = std::max(-1000.0f, std::min(bodies2d[selectedBody].vy, 1000.0f));
            }
            ImGui::End();
        }
        break;
        case sim::States::NBodySmallSim:
            if (trail)
            {
                shaderProgramTrail.use();
                shaderProgramTrail.uniform1i("uMaxIndex", trailLength);
                glBindVertexArray(trailVAO);
                glDrawArrays(GL_POINTS, 0, trailVertices.size());
            }
            shaderProgram.use();
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, numOfBodies);

            update2dBodies(bodies2d);

            if (trail)
            {
                for (int i = 0; i < bodies2d.size(); i++)
                {
                    for (int j = 0; j < trailLength - 1; j++)
                    {
                        trailVertices[i * trailLength + j].x = trailVertices[i * trailLength + j + 1].x;
                        trailVertices[i * trailLength + j].y = trailVertices[i * trailLength + j + 1].y;
                    }
                    trailVertices[(i + 1) * trailLength - 1].x = vertices[i * 2];
                    trailVertices[(i + 1) * trailLength - 1].y = vertices[i * 2 + 1];
                }
            }
            for (int i = 0; i < bodies2d.size(); i++)
            {
                vertices[i * 2] = bodies2d[i].x / 1000.0f;
                vertices[i * 2 + 1] = bodies2d[i].y / 1000.0f;
            }

            if (trail)
            {
                glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
                void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, trailVertices.size() * sizeof(trailStruct), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                if (ptr != NULL)
                {
                    memcpy(ptr, trailVertices.data(), trailVertices.size() * sizeof(trailStruct));
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                }
            }
            {
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                if (ptr != NULL)
                {
                    memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                }
            }
            currentTime = glfwGetTime();
            break;
        case sim::States::NBodyBigInit:
        {
            srand(time(NULL));
            for (int i = 0; i < numOfBodies; i++)
            {
                bodies2d[i].mass = std::max((float)rand() / RAND_MAX, 0.1f) * 10.0f;
                bodies2d[i].x = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 100.0f;
                bodies2d[i].y = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 100.0f;
                bodies2d[i].vx = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 10.0f;
                bodies2d[i].vy = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 10.0f;
            }
            vertices.clear();
            vertices = std::vector<float>(numOfBodies * 2);
            for (int i = 0; i < numOfBodies; i++)
            {
                vertices[i * 2] = bodies2d[i].x / 1000.0f;
                vertices[i * 2 + 1] = bodies2d[i].y / 1000.0f;
            }
            shaderProgram = gui::Shader("resources/shaders/vertexShaders/bigNBodies.ver", "resources/shaders/fragmentShaders/bigNBodies.frag");

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
            currentTime = glfwGetTime();
            state = sim::States::NBodyBigSim;
        }
        break;
        case sim::States::NBodyBigSim:
        {
            shaderProgram.use();
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, numOfBodies);

            sim::QuadTree *qt = new sim::QuadTree(-1000.0, 1000.0, 1000.0, -1000.0);
            for (int i = 0; i < bodies2d.size(); i++)
            {
                qt->addBody(bodies2d[i]);
            }
            double newTime = glfwGetTime();
            for (int i = 0; i < bodies2d.size(); i++)
            {
                std::vector<float> a = qt->calForce(bodies2d[i], G, alpha, theta);
                bodies2d[i].vx += a[0] * (newTime - currentTime) * 10;
                bodies2d[i].vy += a[1] * (newTime - currentTime) * 10;
                bodies2d[i].x += bodies2d[i].vx * (newTime - currentTime);
                bodies2d[i].y += bodies2d[i].vy * (newTime - currentTime);
            }
            for (int i = 0; i < bodies2d.size(); i++)
            {
                vertices[i * 2] = bodies2d[i].x / 1000.0f;
                vertices[i * 2 + 1] = bodies2d[i].y / 1000.0f;
            }
            {
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                if (ptr != NULL)
                {
                    memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                }
            }
            delete qt;
            currentTime = glfwGetTime();
        }
        break;
        case sim::States::ThreeBody3DInit:
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
            ImGui::BeginGroup();
            ImVec2 button_size = ImVec2(140, 60);
            if (ImGui::Button("Start", button_size))
            {
                vertices.clear();
                vertices = std::vector<float>(numOfBodies * 3);
                for (int i = 0; i < 3; i++)
                {
                    vertices[i * 3] = bodies3d[i].x / 1000.0f;
                    vertices[i * 3 + 1] = bodies3d[i].y / 1000.0f;
                    vertices[i * 3 + 2] = bodies3d[i].z / 1000.0f;
                }
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver", "resources/shaders/fragmentShaders/threeBodies2d.frag");

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
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                currentTime = glfwGetTime();
                state = sim::States::ThreeBody3DSim;
            }
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Dummy(ImVec2(100.0f, 0.0f));
            ImGui::SameLine();
            ImGui::BeginGroup();
            std::vector<const char *> inputFloatNames({"mass1", "x1", "y1", "z1", "vx1", "vy1", "vz1",
                                                       "mass2", "x2", "y2", "z2", "vx2", "vy2", "vz2",
                                                       "mass3", "x3", "y3", "z3", "vx3", "vy3", "vz3"});
            for (int i = 0, j = 0; i < 3; i++)
            {
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies3d[i].mass, 0.1f, 1.0f, "%.2f"))
                {
                    bodies3d[i].mass = std::max(0.1f, std::min(bodies3d[i].mass, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies3d[i].x, 0.1f, 1.0f, "%.2f"))
                {
                    bodies3d[i].x = std::max(-1000.0f, std::min(bodies3d[i].x, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies3d[i].y, 0.1f, 1.0f, "%.2f"))
                {
                    bodies3d[i].y = std::max(-1000.0f, std::min(bodies3d[i].y, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies3d[i].z, 0.1f, 1.0f, "%.2f"))
                {
                    bodies3d[i].z = std::max(-1000.0f, std::min(bodies3d[i].z, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies3d[i].vx, 0.1f, 1.0f, "%.2f"))
                {
                    bodies3d[i].vx = std::max(-1000.0f, std::min(bodies3d[i].vx, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies3d[i].vy, 0.1f, 1.0f, "%.2f"))
                {
                    bodies3d[i].vy = std::max(-1000.0f, std::min(bodies3d[i].vy, 1000.0f));
                }
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies3d[i].vz, 0.1f, 1.0f, "%.2f"))
                {
                    bodies3d[i].vz = std::max(-1000.0f, std::min(bodies3d[i].vz, 1000.0f));
                }
            }
            ImGui::EndGroup();
            ImGui::EndGroup();
            ImGui::End();
        }
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

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    shaderProgram.destroy();
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
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
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

void update2dBodies(std::vector<sim::Body2d> &bodies)
{
    double newTime = glfwGetTime();
    double a[bodies.size()][2];
    for (int i = 0; i < bodies.size(); i++)
    {
        a[i][0] = 0, a[i][1] = 0;
        for (int j = 0; j < bodies.size(); j++)
        {
            if (i != j)
            {
                double dx = bodies2d[j].x - bodies2d[i].x;
                double dy = bodies2d[j].y - bodies2d[i].y;
                double distSqr = dx * dx + dy * dy + alpha * alpha;
                double invDist = 1.0 / sqrt(distSqr);
                double invDist3 = invDist * invDist * invDist;

                a[i][0] += G * bodies2d[j].mass * dx * invDist3;
                a[i][1] += G * bodies2d[j].mass * dy * invDist3;
            }
        }
    }
    for (int i = 0; i < bodies.size(); i++)
    {
        bodies2d[i].vx += a[i][0] * (newTime - currentTime) * 10;
        bodies2d[i].vy += a[i][1] * (newTime - currentTime) * 10;
        bodies2d[i].x += bodies2d[i].vx * (newTime - currentTime);
        bodies2d[i].y += bodies2d[i].vy * (newTime - currentTime);
    }
}