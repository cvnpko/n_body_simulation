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
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "simulation/quadTree.hpp"
#include "simulation/body.hpp"
#include "simulation/simulation.hpp"
#include "gui/shader.hpp"
#include "gui/camera.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void update2dBodies(std::vector<sim::Body2d> &bodies);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

void draw(GLFWwindow *window);

struct trailStruct
{
    float x, y;
    int index;
};

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
sim::States state = sim::States::MENU;
sim::Option option = sim::Option::MENU;
std::vector<sim::Body2d> bodies2d;
std::vector<sim::Body3d> bodies3d;
std::vector<float> vertices;
std::vector<trailStruct> trailVertices;
gui::Shader shaderProgram, shaderProgramTrail;
unsigned int VBO = 0, VAO = 0, trailVBO = 0, trailVAO = 0;
double currentTime, deltaTime;
const double G = 6700.0;
const double alpha = 5.0;
bool trail = false;
unsigned int trailLength = 500;
int numOfBodies = 0;
float theta = 2.0f;
glm::mat4 view;
glm::mat4 projection;
gui::Camera camera(SCR_WIDTH, SCR_HEIGHT);

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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize glad\n";
        return 1;
    }
    glEnable(GL_PROGRAM_POINT_SIZE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiStyle &style = ImGui::GetStyle();
    style.FrameRounding = 20.0f;
    ImGui::GetIO().FontGlobalScale = 2.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    currentTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        double newTime = glfwGetTime();
        deltaTime = newTime - currentTime;
        currentTime = newTime;
        draw(window);
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
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.updateKeyboard(GLFW_KEY_W, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.updateKeyboard(GLFW_KEY_S, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.updateKeyboard(GLFW_KEY_D, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.updateKeyboard(GLFW_KEY_A, deltaTime);
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
        if (state == sim::States::Sim)
        {
            state = sim::States::Init;
        }
        else if (state == sim::States::Init)
        {
            state = sim::States::MENU;
        }
    }
}

void update2dBodies(std::vector<sim::Body2d> &bodies)
{
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
        bodies2d[i].vx += a[i][0] * deltaTime;
        bodies2d[i].vy += a[i][1] * deltaTime;
        bodies2d[i].x += bodies2d[i].vx * deltaTime;
        bodies2d[i].y += bodies2d[i].vy * deltaTime;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.updateFov(yoffset);
}
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    camera.updateMouse(xposIn, yposIn);
}
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS)
    {
        camera.updateMouseClick(button);
    }
}

void draw(GLFWwindow *window)
{
    if (state == sim::States::MENU)
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
        std::vector<sim::Option> buttonOption({sim::Option::ThreeBody2D, sim::Option::TwoFixedBody, sim::Option::NBodySmall, sim::Option::NBodyBig, sim::Option::ThreeBody3D});
        ImVec2 button_size = ImVec2(window_size.x, window_size.y / 6.0f);
        ImVec2 dummy_size = ImVec2(window_size.x, window_size.y / 6.0f / 12.0f);
        for (int i = 0; i < buttonNames.size(); i++)
        {
            ImGui::Dummy(dummy_size);
            if (ImGui::Button(buttonNames[i], button_size))
            {
                state = sim::States::Init;
                option = buttonOption[i];
                switch (option)
                {
                case sim::Option::ThreeBody2D:
                case sim::Option::TwoFixedBody:
                    numOfBodies = 3;
                    bodies2d = std::vector<sim::Body2d>(numOfBodies);
                    break;
                case sim::Option::NBodySmall:
                    numOfBodies = 1;
                    bodies2d = std::vector<sim::Body2d>(numOfBodies);
                    break;
                case sim::Option::NBodyBig:
                    numOfBodies = 50000;
                    bodies2d = std::vector<sim::Body2d>(numOfBodies);
                    break;
                case sim::Option::ThreeBody3D:
                    numOfBodies = 3;
                    bodies3d = std::vector<sim::Body3d>(numOfBodies);
                    break;
                }
            }
        }
        ImGui::End();
    }
    else if (state == sim::States::Init)
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
            if (option == sim::Option::NBodyBig)
            {
                srand(time(NULL));
                for (int i = 0; i < numOfBodies; i++)
                {
                    bodies2d[i].mass = std::max((float)rand() / RAND_MAX, 0.1f) * 10.0f;
                    bodies2d[i].x = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 1000.0f;
                    bodies2d[i].y = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 1000.0f;
                    bodies2d[i].vx = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 10.0f;
                    bodies2d[i].vy = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 10.0f;
                }
            }
            vertices.clear();
            if (option == sim::Option::ThreeBody3D)
            {
                vertices = std::vector<float>(numOfBodies * 3);
                for (int i = 0; i < numOfBodies; i++)
                {
                    vertices[i * 3] = bodies3d[i].x / 1000.0f;
                    vertices[i * 3 + 1] = bodies3d[i].y / 1000.0f;
                    vertices[i * 3 + 2] = bodies3d[i].z / 1000.0f;
                }
            }
            else
            {
                vertices = std::vector<float>(numOfBodies * 2);
                for (int i = 0; i < numOfBodies; i++)
                {
                    vertices[i * 2] = bodies2d[i].x / 1000.0f;
                    vertices[i * 2 + 1] = bodies2d[i].y / 1000.0f;
                }
            }
            switch (option)
            {
            case sim::Option::ThreeBody2D:
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver",
                                            "resources/shaders/fragmentShaders/threeBodies2d.frag");
                break;
            case sim::Option::TwoFixedBody:
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver",
                                            "resources/shaders/fragmentShaders/threeBodies2d.frag");
                break;
            case sim::Option::NBodySmall:
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver",
                                            "resources/shaders/fragmentShaders/threeBodies2d.frag");
                break;
            case sim::Option::NBodyBig:
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/bigNBodies.ver",
                                            "resources/shaders/fragmentShaders/bigNBodies.frag");
                break;
            case sim::Option::ThreeBody3D:
                shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies3d.ver",
                                            "resources/shaders/fragmentShaders/threeBodies3d.frag");
                break;
            }

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
            if (option == sim::Option::ThreeBody3D)
            {
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            }
            else
            {
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
            }

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            if (trail)
            {
                trailVertices.clear();
                if (option == sim::Option::TwoFixedBody)
                {
                    trailVertices = std::vector<trailStruct>(trailLength);
                    for (int j = 0; j < trailLength; j++)
                    {
                        trailVertices[j].x = vertices[0];
                        trailVertices[j].y = vertices[1];
                        trailVertices[j].index = j;
                    }
                }
                else
                {
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
            state = sim::States::Sim;
        }
        if (option == sim::Option::ThreeBody2D || option == sim::Option::NBodySmall || option == sim::Option::TwoFixedBody)
        {
            ImGui::Checkbox("Trail", &trail);
        }
        if (option == sim::Option::NBodySmall)
        {
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
        }
        ImGui::EndGroup();
        if (option == sim::Option::ThreeBody2D || option == sim::Option::TwoFixedBody || option == sim::Option::ThreeBody3D)
        {
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Dummy(ImVec2(100.0f, 0.0f));
            ImGui::SameLine();
            ImGui::BeginGroup();
            switch (option)
            {
            case sim::Option::ThreeBody2D:
            {
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
            }
            break;
            case sim::Option::TwoFixedBody:
            {
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
            }
            break;
            case sim::Option::ThreeBody3D:
            {
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
            }
            break;
            }
            ImGui::EndGroup();
            ImGui::EndGroup();
        }
        ImGui::End();
    }
    else if (state == sim::States::Sim)
    {
        switch (option)
        {
        case sim::Option::ThreeBody2D:
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
        }
        break;

        case sim::Option::TwoFixedBody:
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

            bodies2d[0].vx += a[0] * (deltaTime);
            bodies2d[0].vy += a[1] * (deltaTime);
            bodies2d[0].x += bodies2d[0].vx * (deltaTime);
            bodies2d[0].y += bodies2d[0].vy * (deltaTime);

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
        }
        break;

        case sim::Option::NBodySmall:
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
            break;

        case sim::Option::NBodyBig:
        {
            shaderProgram.use();
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, numOfBodies);
            sim::QuadTree *qt = new sim::QuadTree(-1000.0, 1000.0, 1000.0, -1000.0);
            for (int i = 0; i < numOfBodies; i++)
            {
                qt->addBody(bodies2d[i]);
            }
            for (int i = 0; i < numOfBodies; i++)
            {
                std::vector<float> a = qt->calForce(bodies2d[i], G, alpha, theta);
                bodies2d[i].vx += a[0] * deltaTime * 10;
                bodies2d[i].vy += a[1] * deltaTime * 10;
                bodies2d[i].x += bodies2d[i].vx * deltaTime;
                bodies2d[i].y += bodies2d[i].vy * deltaTime;
            }
            for (int i = 0; i < numOfBodies; i++)
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
        }
        break;

        case sim::Option::ThreeBody3D:
        {
            shaderProgram.use();
            projection = glm::mat4(1.0f);
            projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            shaderProgram.uniform4mat("projection", projection);
            glm::mat4 view = camera.lookAt();
            shaderProgram.uniform4mat("view", view);
            glBindVertexArray(VAO);
            glDrawArrays(GL_POINTS, 0, 3);

            double a[bodies3d.size()][3];
            for (int i = 0; i < bodies3d.size(); i++)
            {
                a[i][0] = a[i][1] = a[i][2] = 0;
                for (int j = 0; j < bodies3d.size(); j++)
                {
                    if (i != j)
                    {
                        double dx = bodies3d[j].x - bodies3d[i].x;
                        double dy = bodies3d[j].y - bodies3d[i].y;
                        double dz = bodies3d[j].z - bodies3d[i].z;
                        double distSqr = dx * dx + dy * dy + dz * dz + alpha * alpha;
                        double invDist = 1.0 / sqrt(distSqr);
                        double invDist3 = invDist * invDist * invDist;
                        a[i][0] += G * bodies3d[j].mass * dx * invDist3;
                        a[i][1] += G * bodies3d[j].mass * dy * invDist3;
                        a[i][2] += G * bodies3d[j].mass * dz * invDist3;
                    }
                }
            }
            for (int i = 0; i < bodies3d.size(); i++)
            {
                bodies3d[i].vx += a[i][0] * deltaTime * 10;
                bodies3d[i].vy += a[i][1] * deltaTime * 10;
                bodies3d[i].vz += a[i][2] * deltaTime * 10;
                bodies3d[i].x += bodies3d[i].vx * deltaTime;
                bodies3d[i].y += bodies3d[i].vy * deltaTime;
                bodies3d[i].z += bodies3d[i].vz * deltaTime;
            }
            for (int i = 0; i < bodies3d.size(); i++)
            {
                vertices[i * 3] = bodies3d[i].x / 1000.0f;
                vertices[i * 3 + 1] = bodies3d[i].y / 1000.0f;
                vertices[i * 3 + 2] = bodies3d[i].z / 1000.0f;
            }
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            if (ptr != NULL)
            {
                memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
                glUnmapBuffer(GL_ARRAY_BUFFER);
            }
        }
        break;
        }
    }
    else
    {
        glfwSetWindowShouldClose(window, true);
    }
}