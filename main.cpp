#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
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
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

GLFWimage loadIcon(const char *filename);

void draw(GLFWwindow *window);
void drawMenu();
void drawInit();
void drawInitThreeBody2D();
void drawInitThreeBody3D();
void drawInitTwoFixedBody();
void drawInitNBodySmall();
void drawInitNBodyBig();
void drawSim(GLFWwindow *window);
void drawSimThreeBody2D(GLFWwindow *window);
void drawSimThreeBody3D(GLFWwindow *window);
void drawSimTwoFixedBody(GLFWwindow *window);
void drawSimNBodySmall(GLFWwindow *window);
void drawSimNBodyBig(GLFWwindow *window);

float vectorMagnitude(std::vector<float> &coords);
bool doCirclesOverlap(int dimension, std::vector<float> &coords1, float r1, std::vector<float> &coords2, float r2);
float dotProduct(int dimension, std::vector<float> &coords1, std::vector<float> &coords2);

struct trailStruct
{
    float x, y;
    int index;
};

ImFont *smallFont;
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;
sim::States state = sim::States::MENU;
sim::Option option = sim::Option::MENU;
std::vector<sim::Body> bodies;
int dimension = 0;
std::vector<float> vertices;
const std::vector<float> lineVertices({-100000.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                       0.0f, 0.0f, 0.0f, 1.0f, 100000.0f, 0.0f, 0.0f, 1.0f,
                                       0.0f, -100000.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 2.0f,
                                       0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 100000.0f, 0.0f, 2.0f,
                                       0.0f, 0.0f, -100000.0f, 3.0f, 0.0f, 0.0f, 0.0f, 3.0f,
                                       0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 100000.0f, 3.0f});
std::vector<trailStruct> trailVertices;
gui::Shader shaderProgram, shaderProgramTrail, shaderProgramLine;
unsigned int VBO = 0, VAO = 0, trailVBO = 0, trailVAO = 0, lineVBO = 0, lineVAO = 0;
double currentTime, deltaTime;
float radius;
const double G = 6674;
const double alpha = 5.0;
bool trail = false;
bool walls = false;
bool collisions = false;
bool infos = false;
float restitutionCoeff = 0.0f;
const unsigned int trailLength = 500;
int numOfBodies = 0;
const float theta = 2.0f;
int selectedBody;
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
    GLFWimage icon = loadIcon("resources/images/icon.png");
    glfwSetWindowIcon(window, 1, &icon);

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    stbi_image_free(icon.pixels);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &trailVAO);
    glDeleteBuffers(1, &trailVBO);
    shaderProgram.destroy();
    shaderProgramTrail.destroy();
    shaderProgramLine.destroy();
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
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &VAO);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDeleteBuffers(1, &VBO);
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &lineVAO);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDeleteBuffers(1, &lineVBO);
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &trailVAO);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDeleteBuffers(1, &trailVBO);
        }
        else if (state == sim::States::Init)
        {
            state = sim::States::MENU;
            vertices.clear();
            trailVertices.clear();
            bodies.clear();
            dimension = 0;
            trail = false;
            walls = false;
            infos = false;
            collisions = false;
            radius = 0.0f;
        }
    }
    if (state == sim::States::Sim && option != sim::Option::NBodyBig && key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        infos = !infos;
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
        drawMenu();
    }
    else if (state == sim::States::Init)
    {
        drawInit();
    }
    else if (state == sim::States::Sim)
    {
        drawSim(window);
    }
}

void drawMenu()
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    if (!smallFont)
    {
        smallFont = io.Fonts->AddFontDefault(
            [](ImFontConfig *cfg)
            {
                cfg->SizePixels = 8.0f;
                return cfg;
            }(new ImFontConfig()));
    }
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
    std::vector<const char *> buttonNames({"Three bodies", "Fixed 2 bodies", "Small n bodies", "Big n bodies", "Three bodies 3D"});
    std::vector<sim::Option> buttonOption({sim::Option::ThreeBody2D, sim::Option::TwoFixedBody, sim::Option::NBodySmall, sim::Option::NBodyBig, sim::Option::ThreeBody3D});
    ImVec2 button_size = ImVec2(window_size.x, window_size.y / 6.0f);
    ImVec2 dummy_size = ImVec2(window_size.x, window_size.y / 6.0f / 12.0f);
    ImGui::BeginGroup();
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
                radius = 10.0f;
                numOfBodies = 3;
                dimension = 2;
                break;
            case sim::Option::TwoFixedBody:
                radius = 10.0f;
                numOfBodies = 3;
                dimension = 2;
                break;
            case sim::Option::NBodySmall:
                radius = 10.0f;
                numOfBodies = 1;
                dimension = 2;
                break;
            case sim::Option::NBodyBig:
                radius = 3.0f;
                numOfBodies = 10000;
                dimension = 2;
                break;
            case sim::Option::ThreeBody3D:
                radius = 30.0f;
                numOfBodies = 3;
                dimension = 3;
                break;
            }
            bodies = std::vector<sim::Body>(numOfBodies, sim::Body(dimension));
            switch (option)
            {
            case sim::Option::ThreeBody2D:
                bodies[0].coord[0] = 0.0f;
                bodies[0].coord[1] = 0.0f;
                bodies[0].veloc[0] = 0.0f;
                bodies[0].veloc[1] = 0.0f;
                bodies[0].mass = 1.0f;
                bodies[1].coord[0] = 0.0f;
                bodies[1].coord[1] = 0.0f;
                bodies[1].veloc[0] = 0.0f;
                bodies[1].veloc[1] = 0.0f;
                bodies[1].mass = 1.0f;
                bodies[2].coord[0] = 0.0f;
                bodies[2].coord[1] = 0.0f;
                bodies[2].veloc[0] = 0.0f;
                bodies[2].veloc[1] = 0.0f;
                bodies[2].mass = 1.0f;
                break;
            case sim::Option::TwoFixedBody:
                bodies[0].coord[0] = 0.0f;
                bodies[0].coord[1] = 0.0f;
                bodies[0].veloc[0] = 0.0f;
                bodies[0].veloc[1] = 0.0f;
                bodies[0].mass = 1.0f;
                bodies[1].coord[0] = 0.0f;
                bodies[1].coord[1] = 0.0f;
                bodies[1].mass = 1.0f;
                bodies[2].coord[0] = 0.0f;
                bodies[2].coord[1] = 0.0f;
                bodies[2].mass = 1.0f;
                break;
            case sim::Option::NBodySmall:
                bodies[0].coord[0] = 0.0f;
                bodies[0].coord[1] = 0.0f;
                bodies[0].veloc[0] = 0.0f;
                bodies[0].veloc[1] = 0.0f;
                bodies[0].mass = 1.0f;
                break;
            case sim::Option::NBodyBig:
                srand(time(NULL));
                for (int i = 0; i < numOfBodies; i++)
                {
                    bodies[i].mass = std::max((float)rand() / RAND_MAX, 0.1f) * 10.0f;
                    for (int j = 0; j < dimension; j++)
                    {
                        bodies[i].coord[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 1000.0f;
                        bodies[i].veloc[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 10.0f;
                    }
                }
                break;
            case sim::Option::ThreeBody3D:
                bodies[0].coord[0] = 0.0f;
                bodies[0].coord[1] = 0.0f;
                bodies[0].coord[2] = 0.0f;
                bodies[0].veloc[0] = 0.0f;
                bodies[0].veloc[1] = 0.0f;
                bodies[0].coord[2] = 0.0f;
                bodies[0].mass = 1.0f;
                bodies[1].coord[0] = 0.0f;
                bodies[1].coord[1] = 0.0f;
                bodies[1].coord[2] = 0.0f;
                bodies[1].veloc[0] = 0.0f;
                bodies[1].veloc[1] = 0.0f;
                bodies[1].coord[2] = 0.0f;
                bodies[1].mass = 1.0f;
                bodies[2].coord[0] = 0.0f;
                bodies[2].coord[1] = 0.0f;
                bodies[2].coord[2] = 0.0f;
                bodies[2].veloc[0] = 0.0f;
                bodies[2].veloc[1] = 0.0f;
                bodies[2].coord[2] = 0.0f;
                bodies[2].mass = 1.0f;
                break;
            }
        }
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::PushFont(smallFont);
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(0.0f, window_size.y - 200));
    ImGui::Dummy(ImVec2(30.0f, 0));
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Keybinds: \nPrevious screen: CTRL\nExit: ESC\nInfos: T\nMovement(3D only): WASD\nLock cam(3D only): R.CLICK");
    ImGui::Text("Units:\nLength: 10^11m\nMass: 10^4kg\nTime: 1s sim = 1mo IRL");
    ImGui::EndGroup();
    ImGui::EndGroup();
    ImGui::PopFont();
    ImGui::End();
}

void drawInit()
{
    switch (option)
    {
    case sim::Option::ThreeBody2D:
        drawInitThreeBody2D();
        break;
    case sim::Option::TwoFixedBody:
        drawInitTwoFixedBody();
        break;
    case sim::Option::NBodySmall:
        drawInitNBodySmall();
        break;
    case sim::Option::NBodyBig:
        drawInitNBodyBig();
        break;
    case sim::Option::ThreeBody3D:
        drawInitThreeBody3D();
        break;
    }
}

void drawSim(GLFWwindow *window)
{
    switch (option)
    {
    case sim::Option::NBodyBig:
        drawSimNBodyBig(window);
        break;
    case sim::Option::NBodySmall:
        drawSimNBodySmall(window);
        break;
    case sim::Option::ThreeBody2D:
        drawSimThreeBody2D(window);
        break;
    case sim::Option::ThreeBody3D:
        drawSimThreeBody3D(window);
        break;
    case sim::Option::TwoFixedBody:
        drawSimTwoFixedBody(window);
        break;
    }
}

void drawInitThreeBody2D()
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
    ImVec2 button_size = ImVec2(window_size.x / 4.0f, window_size.y / 16.0f);
    ImGui::SetCursorPos(ImVec2((window_size.x - button_size.x) / 2.0f, 20));
    if (ImGui::Button("Start", button_size))
    {
        vertices = std::vector<float>(numOfBodies * dimension);
        for (int i = 0; i < numOfBodies; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
            }
        }
        shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver",
                                    "resources/shaders/fragmentShaders/threeBodies2d.frag");
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, dimension, GL_FLOAT, GL_FALSE, dimension * sizeof(float), (void *)0);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if (trail)
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
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(window_size.x / 2.25f - button_size.x / 3.5f, 100.0f));
    ImGui::BeginGroup();
    ImGui::Checkbox("Trail", &trail);
    ImGui::Checkbox("Walls", &walls);
    ImGui::Checkbox("Collisions", &collisions);
    std::vector<const char *> inputFloatNames({"mass1", "x1", "y1", "vx1", "vy1",
                                               "mass2", "x2", "y2", "vx2", "vy2",
                                               "mass3", "x3", "y3", "vx3", "vy3"});
    for (int i = 0, j = 0; i < numOfBodies; i++)
    {
        switch (i)
        {
        case 0:
            ImGui::Text("First body:");
            break;
        case 1:
            ImGui::Text("Second body:");
            break;
        case 2:
            ImGui::Text("Third body:");
            break;
        }

        if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].mass, 0.1f, 1.0f, "%.2f"))
        {
            bodies[i].mass = std::max(0.1f, std::min(bodies[i].mass, 1000.0f));
        }
        for (int k = 0; k < dimension; k++)
        {
            if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].coord[k], 0.1f, 1.0f, "%.2f"))
            {
                bodies[i].coord[k] = std::max(-1000.0f, std::min(bodies[i].coord[k], 1000.0f));
            }
        }
        for (int k = 0; k < dimension; k++)
        {
            if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].veloc[k], 0.1f, 1.0f, "%.2f"))
            {
                bodies[i].veloc[k] = std::max(-1000.0f, std::min(bodies[i].veloc[k], 1000.0f));
            }
        }
    }
    if (collisions)
    {
        ImGui::Text("Other:");
        if (ImGui::InputFloat("COR", &restitutionCoeff, 0.1f, 1.0f, "%.2f"))
        {
            restitutionCoeff = std::max(0.0f, std::min(restitutionCoeff, 1.0f));
        }
    }
    ImGui::EndGroup();
    ImGui::EndGroup();
    ImGui::End();
}
void drawInitTwoFixedBody()
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
    ImVec2 button_size = ImVec2(window_size.x / 4.0f, window_size.y / 16.0f);
    ImGui::SetCursorPos(ImVec2((window_size.x - button_size.x) / 2.0f, 20));
    if (ImGui::Button("Start", button_size))
    {
        vertices = std::vector<float>(numOfBodies * dimension);
        for (int i = 0; i < numOfBodies; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
            }
        }
        shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver",
                                    "resources/shaders/fragmentShaders/threeBodies2d.frag");
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, dimension, GL_FLOAT, GL_FALSE, dimension * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        if (trail)
        {
            trailVertices = std::vector<trailStruct>(trailLength);
            for (int j = 0; j < trailLength; j++)
            {
                trailVertices[j].x = vertices[0];
                trailVertices[j].y = vertices[1];
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
        state = sim::States::Sim;
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(window_size.x / 2.25f - button_size.x / 3.5f, 100.0f));
    ImGui::BeginGroup();
    ImGui::Checkbox("Trail", &trail);
    ImGui::Checkbox("Walls", &walls);
    ImGui::Checkbox("Collisions", &collisions);

    ImGui::Text("First body:");
    if (ImGui::InputFloat("mass1", &bodies[0].mass, 0.1f, 1.0f, "%.2f"))
    {
        bodies[0].mass = std::max(0.1f, std::min(bodies[0].mass, 1000.0f));
    }
    if (ImGui::InputFloat("x1", &bodies[0].coord[0], 0.1f, 1.0f, "%.2f"))
    {
        bodies[0].coord[0] = std::max(-1000.0f, std::min(bodies[0].coord[0], 1000.0f));
    }
    if (ImGui::InputFloat("y1", &bodies[0].coord[1], 0.1f, 1.0f, "%.2f"))
    {
        bodies[0].coord[1] = std::max(-1000.0f, std::min(bodies[0].coord[1], 1000.0f));
    }
    if (ImGui::InputFloat("vx1", &bodies[0].veloc[0], 0.1f, 1.0f, "%.2f"))
    {
        bodies[0].veloc[0] = std::max(-1000.0f, std::min(bodies[0].veloc[0], 1000.0f));
    }
    if (ImGui::InputFloat("vy1", &bodies[0].veloc[1], 0.1f, 1.0f, "%.2f"))
    {
        bodies[0].veloc[1] = std::max(-1000.0f, std::min(bodies[0].veloc[1], 1000.0f));
    }
    std::vector<const char *> inputFloatNames({"mass2", "x2", "y2", "mass3", "x3", "y3"});
    for (int i = 1, j = 0; i < 3; i++)
    {
        switch (i)
        {
        case 1:
            ImGui::Text("Second body:");
            break;
        case 2:
            ImGui::Text("Third body:");
            break;
        }
        if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].mass, 0.1f, 1.0f, "%.2f"))
        {
            bodies[i].mass = std::max(0.1f, std::min(bodies[i].mass, 1000.0f));
        }
        for (int k = 0; k < dimension; k++)
        {
            if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].coord[k], 0.1f, 1.0f, "%.2f"))
            {
                bodies[i].coord[k] = std::max(-1000.0f, std::min(bodies[i].coord[k], 1000.0f));
            }
        }
    }
    if (collisions)
    {
        ImGui::Text("Other:");
        if (ImGui::InputFloat("COR", &restitutionCoeff, 0.1f, 1.0f, "%.2f"))
        {
            restitutionCoeff = std::max(0.0f, std::min(restitutionCoeff, 1.0f));
        }
    }
    ImGui::EndGroup();
    ImGui::EndGroup();

    ImGui::End();
}

void drawInitNBodySmall()
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
    ImVec2 button_size = ImVec2(window_size.x / 4.0f, window_size.y / 16.0f);
    ImGui::SetCursorPos(ImVec2((window_size.x - button_size.x) / 2.0f, 20));
    if (ImGui::Button("Start", button_size))
    {
        vertices = std::vector<float>(numOfBodies * dimension);
        for (int i = 0; i < numOfBodies; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
            }
        }
        shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies2d.ver",
                                    "resources/shaders/fragmentShaders/threeBodies2d.frag");
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, dimension, GL_FLOAT, GL_FALSE, dimension * sizeof(float), (void *)0);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if (trail)
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
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(window_size.x / 2.25f - button_size.x / 3.5f, 100.0f));
    ImGui::BeginGroup();
    ImGui::Checkbox("Trail", &trail);
    ImGui::Checkbox("Walls", &walls);
    ImGui::Checkbox("Collisions", &collisions);
    ImGui::Text("Bodies:");
    if (ImGui::InputInt("Number of bodies", &numOfBodies, 1, 3))
    {
        numOfBodies = std::min(10, std::max(numOfBodies, 1));
        while (bodies.size() < numOfBodies)
        {
            bodies.push_back(sim::Body(dimension));
        }
        while (bodies.size() > numOfBodies)
        {
            bodies.pop_back();
        }
        selectedBody = 0;
    }
    if (ImGui::InputInt("Selected body", &selectedBody, 1, 3))
    {
        selectedBody = std::min(numOfBodies - 1, std::max(selectedBody, 0));
    }
    if (ImGui::InputFloat("mass", &bodies[selectedBody].mass, 0.1f, 1.0f, "%.2f"))
    {
        bodies[selectedBody].mass = std::max(0.1f, std::min(bodies[selectedBody].mass, 1000.0f));
    }
    if (ImGui::InputFloat("x", &bodies[selectedBody].coord[0], 0.1f, 1.0f, "%.2f"))
    {
        bodies[selectedBody].coord[0] = std::max(-1000.0f, std::min(bodies[selectedBody].coord[0], 1000.0f));
    }
    if (ImGui::InputFloat("y", &bodies[selectedBody].coord[1], 0.1f, 1.0f, "%.2f"))
    {
        bodies[selectedBody].coord[1] = std::max(-1000.0f, std::min(bodies[selectedBody].coord[1], 1000.0f));
    }
    if (ImGui::InputFloat("vx", &bodies[selectedBody].veloc[0], 0.1f, 1.0f, "%.2f"))
    {
        bodies[selectedBody].veloc[0] = std::max(-1000.0f, std::min(bodies[selectedBody].veloc[0], 1000.0f));
    }
    if (ImGui::InputFloat("vy", &bodies[selectedBody].veloc[1], 0.1f, 1.0f, "%.2f"))
    {
        bodies[selectedBody].veloc[1] = std::max(-1000.0f, std::min(bodies[selectedBody].veloc[1], 1000.0f));
    }
    if (collisions)
    {
        ImGui::Text("Other:");
        if (ImGui::InputFloat("COR", &restitutionCoeff, 0.1f, 1.0f, "%.2f"))
        {
            restitutionCoeff = std::max(0.0f, std::min(restitutionCoeff, 1.0f));
        }
    }
    ImGui::EndGroup();
    ImGui::EndGroup();
    ImGui::End();
}

void drawInitNBodyBig()
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
    ImVec2 button_size = ImVec2(window_size.x / 3.0f, window_size.y / 12.0f);
    ImGui::SetCursorPos(ImVec2((window_size.x - button_size.x) / 2.0f, window_size.y / 2.0f - button_size.y));
    if (ImGui::Button("Start", button_size))
    {
        vertices = std::vector<float>(numOfBodies * dimension);
        for (int i = 0; i < numOfBodies; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
            }
        }
        shaderProgram = gui::Shader("resources/shaders/vertexShaders/bigNBodies.ver",
                                    "resources/shaders/fragmentShaders/bigNBodies.frag");

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, dimension, GL_FLOAT, GL_FALSE, dimension * sizeof(float), (void *)0);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        state = sim::States::Sim;
    }
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2((window_size.x + button_size.x) / 2.75f, window_size.y / 2.0f - button_size.y + 100));
    ImGui::Checkbox("Walls", &walls);
    ImGui::EndGroup();
    ImGui::End();
}

void drawInitThreeBody3D()
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
    ImVec2 button_size = ImVec2(window_size.x / 4.0f, window_size.y / 16.0f);
    ImGui::SetCursorPos(ImVec2((window_size.x - button_size.x) / 2.0f, 20));
    if (ImGui::Button("Start", button_size))
    {
        vertices = std::vector<float>(numOfBodies * dimension);
        for (int i = 0; i < numOfBodies; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
            }
        }

        shaderProgram = gui::Shader("resources/shaders/vertexShaders/threeBodies3d.ver",
                                    "resources/shaders/fragmentShaders/threeBodies3d.frag");
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, dimension, GL_FLOAT, GL_FALSE, dimension * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        shaderProgramLine = gui::Shader("resources/shaders/vertexShaders/line.ver",
                                        "resources/shaders/fragmentShaders/line.frag");
        glGenVertexArrays(1, &lineVAO);
        glGenBuffers(1, &lineVBO);
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(float), lineVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        state = sim::States::Sim;
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(window_size.x / 2.25f - button_size.x / 3.5f, 100.0f));
    ImGui::BeginGroup();
    ImGui::Checkbox("Collisions", &collisions);
    std::vector<const char *> inputFloatNames({"mass1", "x1", "y1", "z1", "vx1", "vy1", "vz1",
                                               "mass2", "x2", "y2", "z2", "vx2", "vy2", "vz2",
                                               "mass3", "x3", "y3", "z3", "vx3", "vy3", "vz3"});

    for (int i = 0, j = 0; i < numOfBodies; i++)
    {
        switch (i)
        {
        case 0:
            ImGui::Text("First body:");
            break;
        case 1:
            ImGui::Text("Second body:");
            break;
        case 2:
            ImGui::Text("Third body:");
            break;
        }

        if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].mass, 0.1f, 1.0f, "%.2f"))
        {
            bodies[i].mass = std::max(0.1f, std::min(bodies[i].mass, 1000.0f));
        }
        for (int k = 0; k < dimension; k++)
        {
            if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].coord[k], 0.1f, 1.0f, "%.2f"))
            {
                bodies[i].coord[k] = std::max(-1000.0f, std::min(bodies[i].coord[k], 1000.0f));
            }
        }
        for (int k = 0; k < dimension; k++)
        {
            if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].veloc[k], 0.1f, 1.0f, "%.2f"))
            {
                bodies[i].veloc[k] = std::max(-1000.0f, std::min(bodies[i].veloc[k], 1000.0f));
            }
        }
    }
    if (collisions)
    {
        ImGui::Text("Other:");
        if (ImGui::InputFloat("COR", &restitutionCoeff, 0.1f, 1.0f, "%.2f"))
        {
            restitutionCoeff = std::max(0.0f, std::min(restitutionCoeff, 1.0f));
        }
    }
    ImGui::EndGroup();
    ImGui::EndGroup();

    ImGui::End();
}

void drawSimThreeBody2D(GLFWwindow *window)
{
    if (infos)
    {
        ImGuiIO &io = ImGui::GetIO();
        ImVec2 window_size = ImVec2(io.DisplaySize.x, io.DisplaySize.y);
        ImVec2 window_pos = ImVec2(0.0f, 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("Infos", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoBackground);
        for (int i = 0; i < bodies.size(); i++)
        {
            ImGui::Text("x%d=%.2f y%d=%.2f\nvx%d=%.2f vy%d=%.2f",
                        i + 1, bodies[i].coord[0],
                        i + 1, bodies[i].coord[1],
                        i + 1, bodies[i].veloc[0],
                        i + 1, bodies[i].veloc[1]);
        }
        ImGui::End();
    }
    if (trail)
    {
        shaderProgramTrail.use();
        shaderProgramTrail.uniform1i("uMaxIndex", trailLength);
        shaderProgramTrail.uniform1f("radius", radius);
        glBindVertexArray(trailVAO);
        glDrawArrays(GL_POINTS, 0, trailVertices.size());
    }
    shaderProgram.use();
    shaderProgram.uniform1f("radius", radius);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, numOfBodies);

    std::vector<std::vector<double>> a(numOfBodies, std::vector<double>(dimension, 0));
    for (int i = 0; i < numOfBodies; i++)
    {
        for (int j = 0; j < numOfBodies; j++)
        {
            if (i != j)
            {
                std::vector<double> dCoord(dimension);
                double distSqr = alpha * alpha;
                for (int k = 0; k < dimension; k++)
                {
                    dCoord[k] = bodies[j].coord[k] - bodies[i].coord[k];
                    distSqr += dCoord[k] * dCoord[k];
                }
                double invDist = 1.0 / sqrt(distSqr);
                double invDist3 = invDist * invDist * invDist;
                for (int k = 0; k < dimension; k++)
                {
                    a[i][k] += G * bodies[j].mass * dCoord[k] * invDist3;
                }
            }
        }
    }
    for (int i = 0; i < numOfBodies; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            bodies[i].veloc[j] += a[i][j] * deltaTime;
            bodies[i].coord[j] += bodies[i].veloc[j] * deltaTime;

            if (walls)
            {
                if (bodies[i].coord[0] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[i].veloc[0] > 0)
                {
                    bodies[i].veloc[0] *= -1;
                }
                else if (bodies[i].coord[0] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[i].veloc[0] < 0)
                {
                    bodies[i].veloc[0] *= -1;
                }
                else if (bodies[i].coord[1] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[i].veloc[1] > 0)
                {
                    bodies[i].veloc[1] *= -1;
                }
                else if (bodies[i].coord[1] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[i].veloc[1] < 0)
                {
                    bodies[i].veloc[1] *= -1;
                }
            }

            if (collisions)
            {
                for (int k = 0; k < numOfBodies; k++)
                {
                    if (k != i && doCirclesOverlap(2, bodies[i].coord, radius, bodies[k].coord, radius))
                    {
                        float mi = bodies[i].mass;
                        float mk = bodies[k].mass;

                        std::vector<float> n(dimension);
                        for (int l = 0; l < dimension; l++)
                        {
                            n[l] = bodies[k].coord[l] - bodies[i].coord[l];
                        }

                        float nMagnitude = vectorMagnitude(n);
                        std::vector<float> nNormal(dimension);
                        for (int l = 0; l < dimension; l++)
                        {
                            nNormal[l] = n[l] / nMagnitude;
                        }

                        if (restitutionCoeff == 1.0)
                        {
                            if (dimension == 2)
                            {
                                std::vector<float> nTangent(2);
                                nTangent[0] = -nNormal[1];
                                nTangent[1] = nNormal[0];

                                float viNormal = dotProduct(2, bodies[i].veloc, nNormal);
                                float viTangent = dotProduct(2, bodies[i].veloc, nTangent);
                                float vkNormal = dotProduct(2, bodies[k].veloc, nNormal);
                                float vkTangent = dotProduct(2, bodies[k].veloc, nTangent);

                                float viNormalNew = (viNormal * (mi - mk) + 2 * mk * vkNormal) / (mi + mk);
                                float vkNormalNew = (vkNormal * (mi - mk) + 2 * mi * viNormal) / (mi + mk);

                                bodies[i].veloc[0] += nNormal[0] * viNormalNew + nTangent[0] * viTangent;
                                bodies[i].veloc[1] += nNormal[1] * viNormalNew + nTangent[1] * viTangent;

                                bodies[k].veloc[0] += nNormal[0] * vkNormalNew + nTangent[0] * vkTangent;
                                bodies[k].veloc[1] += nNormal[1] * vkNormalNew + nTangent[1] * vkTangent;
                            }
                            else
                            {
                                std::vector<float> vRelative(3);
                                for (int l = 0; l < 3; l++)
                                {
                                    vRelative[l] = bodies[i].veloc[l] - bodies[k].veloc[l];
                                }

                                float vRelNormal = dotProduct(3, vRelative, nNormal);
                                float impulse = 2 * vRelNormal / (mi + mk);

                                bodies[i].veloc[0] -= nNormal[0] * impulse * mk;
                                bodies[i].veloc[1] -= nNormal[1] * impulse * mk;
                                bodies[i].veloc[2] -= nNormal[2] * impulse * mk;

                                bodies[k].veloc[0] += nNormal[0] * impulse * mi;
                                bodies[k].veloc[1] += nNormal[1] * impulse * mi;
                                bodies[k].veloc[2] += nNormal[2] * impulse * mi;
                            }
                        }
                        else if (restitutionCoeff == 0.0)
                        {
                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[i].veloc[l] = bodies[k].veloc[l] = (mi * bodies[i].veloc[l] + mk * bodies[k].veloc[l]) / (mi + mk);
                            }
                        }
                        else
                        {
                            std::vector<float> vRelative(dimension);
                            for (int l = 0; l < dimension; l++)
                            {
                                vRelative[l] = bodies[i].veloc[l] - bodies[k].veloc[l];
                            }

                            float vRelNormal = dotProduct(dimension, vRelative, nNormal);
                            float impulse = -(1.0 + restitutionCoeff) * vRelNormal / (1.0 / mi + 1.0 / mk);

                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[i].veloc[l] += impulse / mi * nNormal[l];
                            }

                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[k].veloc[l] -= impulse / mk * nNormal[l];
                            }
                        }

                        break;
                    }
                }
            }

            vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
        }
    }

    if (trail)
    {
        for (int i = 0; i < numOfBodies; i++)
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

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr != NULL)
    {
        memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

void drawSimTwoFixedBody(GLFWwindow *window)
{
    if (infos)
    {
        ImGuiIO &io = ImGui::GetIO();
        ImVec2 window_size = ImVec2(io.DisplaySize.x, io.DisplaySize.y);
        ImVec2 window_pos = ImVec2(0.0f, 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("Infos", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoBackground);
        ImGui::Text("x1=%.2f y1=%.2f\nvx1=%.2f vy1=%.2f",
                    bodies[0].coord[0],
                    bodies[0].coord[1],
                    bodies[0].veloc[0],
                    bodies[0].veloc[1]);
        ImGui::End();
    }
    if (trail)
    {
        shaderProgramTrail.use();
        shaderProgramTrail.uniform1i("uMaxIndex", trailLength);
        shaderProgramTrail.uniform1f("radius", radius);
        glBindVertexArray(trailVAO);
        glDrawArrays(GL_POINTS, 0, trailVertices.size());
    }
    shaderProgram.use();
    shaderProgram.uniform1f("radius", radius);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, numOfBodies);

    double a[2] = {0, 0};
    for (int j = 1; j < numOfBodies; j++)
    {
        double dx = bodies[j].coord[0] - bodies[0].coord[0];
        double dy = bodies[j].coord[1] - bodies[0].coord[1];
        double distSqr = dx * dx + dy * dy + alpha * alpha;
        double invDist = 1.0 / sqrt(distSqr);
        double invDist3 = invDist * invDist * invDist;

        a[0] += G * bodies[j].mass * dx * invDist3;
        a[1] += G * bodies[j].mass * dy * invDist3;
    }
    bodies[0].veloc[0] += a[0] * deltaTime;
    bodies[0].veloc[1] += a[1] * deltaTime;
    bodies[0].coord[0] += bodies[0].veloc[0] * deltaTime;
    bodies[0].coord[1] += bodies[0].veloc[1] * deltaTime;

    if (walls)
    {
        if (bodies[0].coord[0] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[0].veloc[0] > 0)
        {
            bodies[0].veloc[0] *= -1;
        }
        else if (bodies[0].coord[0] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[0].veloc[0] < 0)
        {
            bodies[0].veloc[0] *= -1;
        }
        else if (bodies[0].coord[1] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[0].veloc[1] > 0)
        {
            bodies[0].veloc[1] *= -1;
        }
        else if (bodies[0].coord[1] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[0].veloc[1] < 0)
        {
            bodies[0].veloc[1] *= -1;
        }
    }

    if (collisions)
    {
        for (int k = 1; k < numOfBodies; k++)
        {
            if (doCirclesOverlap(2, bodies[0].coord, radius, bodies[k].coord, radius))
            {
                float mi = bodies[0].mass;
                float mk = bodies[k].mass;

                std::vector<float> n(2);
                n[0] = bodies[k].coord[0] - bodies[0].coord[0];
                n[1] = bodies[k].coord[1] - bodies[0].coord[1];

                float nMagnitude = vectorMagnitude(n);
                std::vector<float> nNormal(2);
                nNormal[0] = n[0] / nMagnitude;
                nNormal[1] = n[1] / nMagnitude;

                if (restitutionCoeff == 1.0)
                {
                    std::vector<float> nTangent(2);
                    nTangent[0] = -nNormal[1];
                    nTangent[1] = nNormal[0];

                    float viNormal = dotProduct(2, bodies[0].veloc, nNormal);
                    float viTangent = dotProduct(2, bodies[0].veloc, nTangent);
                    float vkNormal = dotProduct(2, bodies[k].veloc, nNormal);
                    float vkTangent = dotProduct(2, bodies[k].veloc, nTangent);

                    float viNormalNew = (viNormal * (mi - mk) + 2 * mk * vkNormal) / (mi + mk);
                    float vkNormalNew = (vkNormal * (mi - mk) + 2 * mi * viNormal) / (mi + mk);

                    bodies[0].veloc[0] += nNormal[0] * viNormalNew + nTangent[0] * viTangent;
                    bodies[0].veloc[1] += nNormal[1] * viNormalNew + nTangent[1] * viTangent;
                }
                else if (restitutionCoeff == 0.0)
                {
                    bodies[0].veloc[0] = 0;
                    bodies[0].veloc[1] = 0;
                }
                else
                {
                    std::vector<float> vRelative(2);
                    vRelative[0] = bodies[0].veloc[0] - bodies[k].veloc[0];
                    vRelative[1] = bodies[0].veloc[1] - bodies[k].veloc[1];

                    float vRelNormal = dotProduct(2, vRelative, nNormal);
                    float impulse = -(1.0 + restitutionCoeff) * vRelNormal / (1.0 / mi + 1.0 / mk);

                    bodies[0].veloc[0] += impulse / mi * nNormal[0];
                    bodies[0].veloc[1] += impulse / mi * nNormal[1];
                }

                break;
            }
        }
    }

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
    vertices[0] = bodies[0].coord[0] / 1000.0f;
    vertices[1] = bodies[0].coord[1] / 1000.0f;

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
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr != NULL)
    {
        memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

void drawSimNBodySmall(GLFWwindow *window)
{
    if (infos)
    {
        ImGuiIO &io = ImGui::GetIO();
        ImVec2 window_size = ImVec2(io.DisplaySize.x, io.DisplaySize.y);
        ImVec2 window_pos = ImVec2(0.0f, 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("Infos", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoBackground);
        for (int i = 0; i < bodies.size(); i++)
        {
            ImGui::Text("x%d=%.2f y%d=%.2f\nvx%d=%.2f vy%d=%.2f",
                        i + 1, bodies[i].coord[0],
                        i + 1, bodies[i].coord[1],
                        i + 1, bodies[i].veloc[0],
                        i + 1, bodies[i].veloc[1]);
        }
        ImGui::End();
    }
    if (trail)
    {
        shaderProgramTrail.use();
        shaderProgramTrail.uniform1i("uMaxIndex", trailLength);
        shaderProgramTrail.uniform1f("radius", radius);
        glBindVertexArray(trailVAO);
        glDrawArrays(GL_POINTS, 0, trailVertices.size());
    }
    shaderProgram.use();
    shaderProgram.uniform1f("radius", radius);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, numOfBodies);

    std::vector<std::vector<double>> a(numOfBodies, std::vector<double>(dimension, 0));
    for (int i = 0; i < numOfBodies; i++)
    {
        for (int j = 0; j < numOfBodies; j++)
        {
            if (i != j)
            {
                std::vector<double> dCoord(dimension);
                double distSqr = alpha * alpha;
                for (int k = 0; k < dimension; k++)
                {
                    dCoord[k] = bodies[j].coord[k] - bodies[i].coord[k];
                    distSqr += dCoord[k] * dCoord[k];
                }
                double invDist = 1.0 / sqrt(distSqr);
                double invDist3 = invDist * invDist * invDist;
                for (int k = 0; k < dimension; k++)
                {
                    a[i][k] += G * bodies[j].mass * dCoord[k] * invDist3;
                }
            }
        }
    }
    for (int i = 0; i < numOfBodies; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            bodies[i].veloc[j] += a[i][j] * deltaTime;
            bodies[i].coord[j] += bodies[i].veloc[j] * deltaTime;

            if (walls)
            {
                if (bodies[i].coord[0] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[i].veloc[0] > 0)
                {
                    bodies[i].veloc[0] *= -1;
                }
                else if (bodies[i].coord[0] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[i].veloc[0] < 0)
                {
                    bodies[i].veloc[0] *= -1;
                }
                else if (bodies[i].coord[1] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[i].veloc[1] > 0)
                {
                    bodies[i].veloc[1] *= -1;
                }
                else if (bodies[i].coord[1] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[i].veloc[1] < 0)
                {
                    bodies[i].veloc[1] *= -1;
                }
            }

            if (collisions)
            {
                for (int k = 0; k < numOfBodies; k++)
                {
                    if (k != i && doCirclesOverlap(2, bodies[i].coord, radius, bodies[k].coord, radius))
                    {
                        float mi = bodies[i].mass;
                        float mk = bodies[k].mass;

                        std::vector<float> n(dimension);
                        for (int l = 0; l < dimension; l++)
                        {
                            n[l] = bodies[k].coord[l] - bodies[i].coord[l];
                        }

                        float nMagnitude = vectorMagnitude(n);
                        std::vector<float> nNormal(dimension);
                        for (int l = 0; l < dimension; l++)
                        {
                            nNormal[l] = n[l] / nMagnitude;
                        }

                        if (restitutionCoeff == 1.0)
                        {
                            if (dimension == 2)
                            {
                                std::vector<float> nTangent(2);
                                nTangent[0] = -nNormal[1];
                                nTangent[1] = nNormal[0];

                                float viNormal = dotProduct(2, bodies[i].veloc, nNormal);
                                float viTangent = dotProduct(2, bodies[i].veloc, nTangent);
                                float vkNormal = dotProduct(2, bodies[k].veloc, nNormal);
                                float vkTangent = dotProduct(2, bodies[k].veloc, nTangent);

                                float viNormalNew = (viNormal * (mi - mk) + 2 * mk * vkNormal) / (mi + mk);
                                float vkNormalNew = (vkNormal * (mi - mk) + 2 * mi * viNormal) / (mi + mk);

                                bodies[i].veloc[0] += nNormal[0] * viNormalNew + nTangent[0] * viTangent;
                                bodies[i].veloc[1] += nNormal[1] * viNormalNew + nTangent[1] * viTangent;

                                bodies[k].veloc[0] += nNormal[0] * vkNormalNew + nTangent[0] * vkTangent;
                                bodies[k].veloc[1] += nNormal[1] * vkNormalNew + nTangent[1] * vkTangent;
                            }
                            else
                            {
                                std::vector<float> vRelative(3);
                                for (int l = 0; l < 3; l++)
                                {
                                    vRelative[l] = bodies[i].veloc[l] - bodies[k].veloc[l];
                                }

                                float vRelNormal = dotProduct(3, vRelative, nNormal);
                                float impulse = 2 * vRelNormal / (mi + mk);

                                bodies[i].veloc[0] -= nNormal[0] * impulse * mk;
                                bodies[i].veloc[1] -= nNormal[1] * impulse * mk;
                                bodies[i].veloc[2] -= nNormal[2] * impulse * mk;

                                bodies[k].veloc[0] += nNormal[0] * impulse * mi;
                                bodies[k].veloc[1] += nNormal[1] * impulse * mi;
                                bodies[k].veloc[2] += nNormal[2] * impulse * mi;
                            }
                        }
                        else if (restitutionCoeff == 0.0)
                        {
                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[i].veloc[l] = bodies[k].veloc[l] = (mi * bodies[i].veloc[l] + mk * bodies[k].veloc[l]) / (mi + mk);
                            }
                        }
                        else
                        {
                            std::vector<float> vRelative(dimension);
                            for (int l = 0; l < dimension; l++)
                            {
                                vRelative[l] = bodies[i].veloc[l] - bodies[k].veloc[l];
                            }

                            float vRelNormal = dotProduct(dimension, vRelative, nNormal);
                            float impulse = -(1.0 + restitutionCoeff) * vRelNormal / (1.0 / mi + 1.0 / mk);

                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[i].veloc[l] += impulse / mi * nNormal[l];
                            }

                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[k].veloc[l] -= impulse / mk * nNormal[l];
                            }
                        }

                        break;
                    }
                }
            }

            vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
        }
    }

    if (trail)
    {
        for (int i = 0; i < numOfBodies; i++)
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
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr != NULL)
    {
        memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

void drawSimNBodyBig(GLFWwindow *window)
{
    shaderProgram.use();
    shaderProgram.uniform1f("radius", radius);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, numOfBodies);

    sim::QuadTree *qt = new sim::QuadTree(-1000.0, 1000.0, 1000.0, -1000.0);
    for (int i = 0; i < numOfBodies; i++)
    {
        qt->addBody(bodies[i]);
    }
    for (int i = 0; i < numOfBodies; i++)
    {
        std::vector<float> a = qt->calForce(bodies[i], G, alpha, theta);
        for (int j = 0; j < dimension; j++)
        {
            bodies[i].veloc[j] += a[j] * deltaTime;
            bodies[i].coord[j] += bodies[i].veloc[j] * deltaTime;
            if (walls)
            {
                if (bodies[i].coord[0] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[i].veloc[0] > 0)
                {
                    bodies[i].veloc[0] *= -1;
                }
                else if (bodies[i].coord[0] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[i].veloc[0] < 0)
                {
                    bodies[i].veloc[0] *= -1;
                }
                else if (bodies[i].coord[1] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[i].veloc[1] > 0)
                {
                    bodies[i].veloc[1] *= -1;
                }
                else if (bodies[i].coord[1] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[i].veloc[1] < 0)
                {
                    bodies[i].veloc[1] *= -1;
                }
            }
        }
    }
    for (int i = 0; i < numOfBodies; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
        }
    }
    delete qt;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr != NULL)
    {
        memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

void drawSimThreeBody3D(GLFWwindow *window)
{
    if (infos)
    {
        ImGuiIO &io = ImGui::GetIO();
        ImVec2 window_size = ImVec2(io.DisplaySize.x, io.DisplaySize.y);
        ImVec2 window_pos = ImVec2(0.0f, 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("Infos", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoBackground);
        glm::vec3 cameraPos = camera.getCameraPos();
        ImGui::Text("xCamera=%.2f yCamera=%.2f zCamera=%.2f",
                    cameraPos.x,
                    cameraPos.y,
                    cameraPos.z);
        for (int i = 0; i < bodies.size(); i++)
        {
            ImGui::Text("x%d=%.2f y%d=%.2f z%d=%.2f\nvx%d=%.2f vy%d=%.2f vz%d=%.2f",
                        i + 1, bodies[i].coord[0],
                        i + 1, bodies[i].coord[1],
                        i + 1, bodies[i].coord[2],
                        i + 1, bodies[i].veloc[0],
                        i + 1, bodies[i].veloc[1],
                        i + 1, bodies[i].veloc[2]);
        }
        ImGui::End();
    }
    projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.lookAt();
    shaderProgramLine.use();
    shaderProgramLine.uniform4mat("projection", projection);
    shaderProgramLine.uniform4mat("view", view);
    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINES, 0, 12);

    shaderProgram.use();
    shaderProgram.uniform1f("radius", radius);
    shaderProgram.uniform4mat("projection", projection);
    shaderProgram.uniform4mat("view", view);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, numOfBodies);

    std::vector<std::vector<double>> a(numOfBodies, std::vector<double>(dimension, 0));
    for (int i = 0; i < numOfBodies; i++)
    {
        for (int j = 0; j < numOfBodies; j++)
        {
            if (i != j)
            {
                std::vector<double> dCoord(dimension);
                double distSqr = alpha * alpha;
                for (int k = 0; k < dimension; k++)
                {
                    dCoord[k] = bodies[j].coord[k] - bodies[i].coord[k];
                    distSqr += dCoord[k] * dCoord[k];
                }
                double invDist = 1.0 / sqrt(distSqr);
                double invDist3 = invDist * invDist * invDist;
                for (int k = 0; k < dimension; k++)
                {
                    a[i][k] += G * bodies[j].mass * dCoord[k] * invDist3;
                }
            }
        }
    }
    for (int i = 0; i < numOfBodies; i++)
    {
        for (int j = 0; j < dimension; j++)
        {
            bodies[i].veloc[j] += a[i][j] * deltaTime;
            bodies[i].coord[j] += bodies[i].veloc[j] * deltaTime;

            if (walls)
            {
                if (bodies[i].coord[0] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[i].veloc[0] > 0)
                {
                    bodies[i].veloc[0] *= -1;
                }
                else if (bodies[i].coord[0] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[i].veloc[0] < 0)
                {
                    bodies[i].veloc[0] *= -1;
                }
                else if (bodies[i].coord[1] + radius > ImGui::GetWindowWidth() * 2.5 && bodies[i].veloc[1] > 0)
                {
                    bodies[i].veloc[1] *= -1;
                }
                else if (bodies[i].coord[1] - radius < ImGui::GetWindowWidth() * -2.5 && bodies[i].veloc[1] < 0)
                {
                    bodies[i].veloc[1] *= -1;
                }
            }

            if (collisions)
            {
                for (int k = 0; k < numOfBodies; k++)
                {
                    if (k != i && doCirclesOverlap(2, bodies[i].coord, radius, bodies[k].coord, radius))
                    {
                        float mi = bodies[i].mass;
                        float mk = bodies[k].mass;

                        std::vector<float> n(dimension);
                        for (int l = 0; l < dimension; l++)
                        {
                            n[l] = bodies[k].coord[l] - bodies[i].coord[l];
                        }

                        float nMagnitude = vectorMagnitude(n);
                        std::vector<float> nNormal(dimension);
                        for (int l = 0; l < dimension; l++)
                        {
                            nNormal[l] = n[l] / nMagnitude;
                        }

                        if (restitutionCoeff == 1.0)
                        {
                            if (dimension == 2)
                            {
                                std::vector<float> nTangent(2);
                                nTangent[0] = -nNormal[1];
                                nTangent[1] = nNormal[0];

                                float viNormal = dotProduct(2, bodies[i].veloc, nNormal);
                                float viTangent = dotProduct(2, bodies[i].veloc, nTangent);
                                float vkNormal = dotProduct(2, bodies[k].veloc, nNormal);
                                float vkTangent = dotProduct(2, bodies[k].veloc, nTangent);

                                float viNormalNew = (viNormal * (mi - mk) + 2 * mk * vkNormal) / (mi + mk);
                                float vkNormalNew = (vkNormal * (mi - mk) + 2 * mi * viNormal) / (mi + mk);

                                bodies[i].veloc[0] += nNormal[0] * viNormalNew + nTangent[0] * viTangent;
                                bodies[i].veloc[1] += nNormal[1] * viNormalNew + nTangent[1] * viTangent;

                                bodies[k].veloc[0] += nNormal[0] * vkNormalNew + nTangent[0] * vkTangent;
                                bodies[k].veloc[1] += nNormal[1] * vkNormalNew + nTangent[1] * vkTangent;
                            }
                            else
                            {
                                std::vector<float> vRelative(3);
                                for (int l = 0; l < 3; l++)
                                {
                                    vRelative[l] = bodies[i].veloc[l] - bodies[k].veloc[l];
                                }

                                float vRelNormal = dotProduct(3, vRelative, nNormal);
                                float impulse = 2 * vRelNormal / (mi + mk);

                                bodies[i].veloc[0] -= nNormal[0] * impulse * mk;
                                bodies[i].veloc[1] -= nNormal[1] * impulse * mk;
                                bodies[i].veloc[2] -= nNormal[2] * impulse * mk;

                                bodies[k].veloc[0] += nNormal[0] * impulse * mi;
                                bodies[k].veloc[1] += nNormal[1] * impulse * mi;
                                bodies[k].veloc[2] += nNormal[2] * impulse * mi;
                            }
                        }
                        else if (restitutionCoeff == 0.0)
                        {
                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[i].veloc[l] = bodies[k].veloc[l] = (mi * bodies[i].veloc[l] + mk * bodies[k].veloc[l]) / (mi + mk);
                            }
                        }
                        else
                        {
                            std::vector<float> vRelative(dimension);
                            for (int l = 0; l < dimension; l++)
                            {
                                vRelative[l] = bodies[i].veloc[l] - bodies[k].veloc[l];
                            }

                            float vRelNormal = dotProduct(dimension, vRelative, nNormal);
                            float impulse = -(1.0 + restitutionCoeff) * vRelNormal / (1.0 / mi + 1.0 / mk);

                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[i].veloc[l] += impulse / mi * nNormal[l];
                            }

                            for (int l = 0; l < dimension; l++)
                            {
                                bodies[k].veloc[l] -= impulse / mk * nNormal[l];
                            }
                        }

                        break;
                    }
                }
            }

            vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr != NULL)
    {
        memcpy(ptr, vertices.data(), vertices.size() * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

float vectorMagnitude(std::vector<float> &coords)
{
    float result = 0.0;
    for (int i = 0; i < coords.size(); i++)
    {
        result += coords[i] * coords[i];
    }
    return result;
}

bool doCirclesOverlap(int dimension, std::vector<float> &coords1, float r1, std::vector<float> &coords2, float r2)
{

    float distance = 0.0f;
    for (int i = 0; i < dimension; i++)
    {
        distance += (coords1[i] - coords2[i]) * (coords1[i] - coords2[i]);
    }

    return distance <= (r1 + r2) * (r1 + r2);
}

float dotProduct(int dimension, std::vector<float> &coords1, std::vector<float> &coords2)
{
    float result = 0;
    for (int i = 0; i < dimension; i++)
    {
        result += coords1[i] * coords2[i];
    }
    return result;
}
GLFWimage loadIcon(const char *filename)
{
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 4); // Load image with RGBA channels

    GLFWimage icon;
    icon.width = width;
    icon.height = height;
    icon.pixels = data;

    return icon;
}
