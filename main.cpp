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
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

void draw(GLFWwindow *window);
void drawMenu();
void drawInit();
void drawSim(GLFWwindow *window);

float vectorMagnitude(std::vector<float> &coords);
bool doCirclesOverlap(int dimension, std::vector<float> &coords1, float r1, std::vector<float> &coords2, float r2);
float dotProduct(int dimension, std::vector<float> &coords1, std::vector<float> &coords2);

struct trailStruct
{
    float x, y;
    int index;
};

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
sim::States state = sim::States::MENU;
sim::Option option = sim::Option::MENU;
std::vector<sim::Body> bodies;
int dimension = 0;
std::vector<float> vertices;
std::vector<trailStruct> trailVertices;
gui::Shader shaderProgram, shaderProgramTrail;
unsigned int VBO = 0, VAO = 0, trailVBO = 0, trailVAO = 0;
double currentTime, deltaTime;
float radius;
const double G = 66740;
const double alpha = 5.0;
bool trail = false;
bool walls = false;
bool collisions = false;
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
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &VAO);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDeleteBuffers(1, &VBO);
        }
        else if (state == sim::States::Init)
        {
            state = sim::States::MENU;
            vertices.clear();
            trailVertices.clear();
            bodies.clear();
            dimension = 0;
            trail = false;
            radius = 0.0f;
        }
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
            dimension = 2;
            radius = 10.0f;
            switch (option)
            {
            case sim::Option::ThreeBody2D:
            case sim::Option::TwoFixedBody:
                numOfBodies = 3;
                break;
            case sim::Option::NBodySmall:
                numOfBodies = 1;
                break;
            case sim::Option::NBodyBig:
                radius = 2.0f;
                numOfBodies = 5000;
                break;
            case sim::Option::ThreeBody3D:
                radius = 300.0f;
                numOfBodies = 3;
                dimension = 3;
                break;
            }
            bodies = std::vector<sim::Body>(numOfBodies, sim::Body(dimension));
        }
    }
    ImGui::End();
}
void drawInit()
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
                bodies[i].mass = std::max((float)rand() / RAND_MAX, 0.1f) * 1.0f;
                for (int j = 0; j < dimension; j++)
                {
                    bodies[i].coord[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 1000.0f;
                    bodies[i].veloc[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * 10.0f;
                }
            }
        }
        vertices = std::vector<float>(numOfBodies * dimension);
        for (int i = 0; i < numOfBodies; i++)
        {
            for (int j = 0; j < dimension; j++)
            {
                vertices[i * dimension + j] = bodies[i].coord[j] / 1000.0f;
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
    if (option == sim::Option::ThreeBody2D || option == sim::Option::NBodySmall || option == sim::Option::TwoFixedBody || option == sim::Option::NBodyBig)
    {
        ImGui::Checkbox("Walls", &walls);
    }
    if (option == sim::Option::ThreeBody2D || option == sim::Option::NBodySmall || option == sim::Option::TwoFixedBody || option == sim::Option::ThreeBody3D)
    {
        ImGui::Checkbox("Collisions", &collisions);
    }
    if (option == sim::Option::NBodySmall)
    {
        ImGui::Text("Bodies:");
        ImGui::SetCursorPosX(40);
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
        ImGui::SetCursorPosX(40);
        if (ImGui::InputInt("Selected body", &selectedBody, 1, 3))
        {
            selectedBody = std::min(numOfBodies - 1, std::max(selectedBody, 0));
        }
        ImGui::SetCursorPosX(40);
        if (ImGui::InputFloat("mass", &bodies[selectedBody].mass, 0.1f, 1.0f, "%.2f"))
        {
            bodies[selectedBody].mass = std::max(0.1f, std::min(bodies[selectedBody].mass, 1000.0f));
        }
        ImGui::SetCursorPosX(40);
        if (ImGui::InputFloat("x", &bodies[selectedBody].coord[0], 0.1f, 1.0f, "%.2f"))
        {
            bodies[selectedBody].coord[0] = std::max(-1000.0f, std::min(bodies[selectedBody].coord[0], 1000.0f));
        }
        ImGui::SetCursorPosX(40);
        if (ImGui::InputFloat("y", &bodies[selectedBody].coord[1], 0.1f, 1.0f, "%.2f"))
        {
            bodies[selectedBody].coord[1] = std::max(-1000.0f, std::min(bodies[selectedBody].coord[1], 1000.0f));
        }
        ImGui::SetCursorPosX(40);
        if (ImGui::InputFloat("vx", &bodies[selectedBody].veloc[0], 0.1f, 1.0f, "%.2f"))
        {
            bodies[selectedBody].veloc[0] = std::max(-1000.0f, std::min(bodies[selectedBody].veloc[0], 1000.0f));
        }
        ImGui::SetCursorPosX(40);
        if (ImGui::InputFloat("vy", &bodies[selectedBody].veloc[1], 0.1f, 1.0f, "%.2f"))
        {
            bodies[selectedBody].veloc[1] = std::max(-1000.0f, std::min(bodies[selectedBody].veloc[1], 1000.0f));
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
        if (option == sim::Option::ThreeBody3D || option == sim::Option::ThreeBody2D)
        {
            std::vector<const char *> inputFloatNames;
            if (option == sim::Option::ThreeBody3D)
            {
                inputFloatNames = std::vector<const char *>({"mass1", "x1", "y1", "z1", "vx1", "vy1", "vz1",
                                                             "mass2", "x2", "y2", "z2", "vx2", "vy2", "vz2",
                                                             "mass3", "x3", "y3", "z3", "vx3", "vy3", "vz3"});
            }
            else
            {
                inputFloatNames = std::vector<const char *>({"mass1", "x1", "y1", "vx1", "vy1",
                                                             "mass2", "x2", "y2", "vx2", "vy2",
                                                             "mass3", "x3", "y3", "vx3", "vy3"});
            }
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

                ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].mass, 0.1f, 1.0f, "%.2f"))
                {
                    bodies[i].mass = std::max(0.1f, std::min(bodies[i].mass, 1000.0f));
                }
                for (int k = 0; k < dimension; k++)
                {
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
                    if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].coord[k], 0.1f, 1.0f, "%.2f"))
                    {
                        bodies[i].coord[k] = std::max(-1000.0f, std::min(bodies[i].coord[k], 1000.0f));
                    }
                }
                for (int k = 0; k < dimension; k++)
                {
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
                    if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].veloc[k], 0.1f, 1.0f, "%.2f"))
                    {
                        bodies[i].veloc[k] = std::max(-1000.0f, std::min(bodies[i].veloc[k], 1000.0f));
                    }
                }
            }
        }
        else if (option == sim::Option::TwoFixedBody)
        {
            ImGui::Text("First body:");
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
            if (ImGui::InputFloat("mass1", &bodies[0].mass, 0.1f, 1.0f, "%.2f"))
            {
                bodies[0].mass = std::max(0.1f, std::min(bodies[0].mass, 1000.0f));
            }
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
            if (ImGui::InputFloat("x1", &bodies[0].coord[0], 0.1f, 1.0f, "%.2f"))
            {
                bodies[0].coord[0] = std::max(-1000.0f, std::min(bodies[0].coord[0], 1000.0f));
            }
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
            if (ImGui::InputFloat("y1", &bodies[0].coord[1], 0.1f, 1.0f, "%.2f"))
            {
                bodies[0].coord[1] = std::max(-1000.0f, std::min(bodies[0].coord[1], 1000.0f));
            }
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
            if (ImGui::InputFloat("vx1", &bodies[0].veloc[0], 0.1f, 1.0f, "%.2f"))
            {
                bodies[0].veloc[0] = std::max(-1000.0f, std::min(bodies[0].veloc[0], 1000.0f));
            }
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
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
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
                if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].mass, 0.1f, 1.0f, "%.2f"))
                {
                    bodies[i].mass = std::max(0.1f, std::min(bodies[i].mass, 1000.0f));
                }
                for (int k = 0; k < dimension; k++)
                {
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 100);
                    if (ImGui::InputFloat(inputFloatNames[j++], &bodies[i].coord[k], 0.1f, 1.0f, "%.2f"))
                    {
                        bodies[i].coord[k] = std::max(-1000.0f, std::min(bodies[i].coord[k], 1000.0f));
                    }
                }
            }
        }
        ImGui::EndGroup();
        ImGui::EndGroup();
    }
    ImGui::End();
}
void drawSim(GLFWwindow *window)
{
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

    if (option == sim::Option::ThreeBody3D)
    {
        projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.getFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shaderProgram.uniform4mat("projection", projection);
        glm::mat4 view = camera.lookAt();
        shaderProgram.uniform4mat("view", view);
    }

    if (option == sim::Option::NBodyBig)
    {
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
                    if (abs(bodies[i].coord[j]) >= ImGui::GetWindowWidth() * 2.5)
                    {
                        bodies[i].veloc[j] *= -1;
                    }
                    else if (abs(bodies[i].coord[j]) >= ImGui::GetWindowHeight() * 2.5)
                    {
                        bodies[i].veloc[j] *= -1;
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
    }
    else if (option == sim::Option::TwoFixedBody)
    {
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
            if (abs(bodies[0].coord[0]) >= ImGui::GetWindowWidth() * 2.5)
            {
                bodies[0].veloc[0] *= -1;
            }
            else if (abs(bodies[0].coord[1]) >= ImGui::GetWindowHeight() * 2.5)
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
                    float vix = bodies[0].veloc[0];
                    float viy = bodies[0].veloc[1];
                    float mi = bodies[0].mass;

                    float vkx = bodies[k].veloc[0];
                    float vky = bodies[k].veloc[1];
                    float mk = bodies[k].mass;

                    bodies[0].veloc[0] = ((mi - mk) * vix + 2 * mk * vkx) / (mi + mk);
                    bodies[0].veloc[1] = ((mi - mk) * viy + 2 * mk * vky) / (mi + mk);

                    bodies[k].veloc[0] = ((mi - mk) * vkx + 2 * mi * vix) / (mi + mk);
                    bodies[k].veloc[1] = ((mi - mk) * vky + 2 * mi * viy) / (mi + mk);

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
    }
    else
    {
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
                    if (abs(bodies[i].coord[j]) >= ImGui::GetWindowWidth() * 2.5)
                    {
                        bodies[i].veloc[j] *= -1;
                    }
                    else if (abs(bodies[i].coord[j]) >= ImGui::GetWindowHeight() * 2.5)
                    {
                        bodies[i].veloc[j] *= -1;
                    }
                }

                if (collisions)
                {
                    for (int k = 0; k < numOfBodies; k++)
                    {
                        if (k != i && doCirclesOverlap(2, bodies[i].coord, radius, bodies[k].coord, radius))
                        {
                            if (dimension == 2)
                            {
                                float mi = bodies[i].mass;
                                float mk = bodies[k].mass;

                                std::vector<float> n(2);
                                for (int l = 0; l < 2; l++)
                                {
                                    n[l] = bodies[k].coord[l] - bodies[i].coord[l];
                                }

                                float nMagnitude = vectorMagnitude(n);
                                std::vector<float> nNormal(2);
                                for (int l = 0; l < 2; l++)
                                {
                                    nNormal[l] = n[l] / nMagnitude;
                                }
                                std::vector<float> nTangent(2);
                                nTangent[0] = -nNormal[1];
                                nTangent[1] = nNormal[0];

                                float viNormal = dotProduct(2, bodies[i].veloc, nNormal);
                                float viTangent = dotProduct(2, bodies[i].veloc, nTangent);
                                float vkNormal = dotProduct(2, bodies[k].veloc, nNormal);
                                float vkTangent = dotProduct(2, bodies[k].veloc, nTangent);

                                float viNormalNew = (viNormal * (mi - mk) + 2 * mk * vkNormal) / (mi + mk);
                                float vkNormalNew = (vkNormal * (mi - mk) + 2 * mi * viNormal) / (mi + mk);

                                bodies[i].veloc[0] = nNormal[0] * viNormalNew + nTangent[0] * viTangent;
                                bodies[i].veloc[1] = nNormal[1] * viNormalNew + nTangent[1] * viTangent;

                                bodies[k].veloc[0] = nNormal[0] * vkNormalNew + nTangent[0] * vkTangent;
                                bodies[k].veloc[1] = nNormal[1] * vkNormalNew + nTangent[1] * vkTangent;
                            }
                            else
                            {
                                float mi = bodies[i].mass;
                                float mk = bodies[k].mass;

                                std::vector<float> n(3);
                                for (int l = 0; l < 3; l++)
                                {
                                    n[l] = bodies[k].coord[l] - bodies[i].coord[l];
                                }

                                float nMagnitude = vectorMagnitude(n);
                                std::vector<float> nNormal(3);
                                for (int l = 0; l < 3; l++)
                                {
                                    nNormal[l] = n[l] / nMagnitude;
                                }

                                std::vector<float> vRelative(3);
                                for (int l = 0; l < 3; l++)
                                {
                                    vRelative[l] = bodies[i].veloc[l] - bodies[k].veloc[l];
                                }

                                float vNormal = dotProduct(3, vRelative, nNormal);
                                float impulse = 2 * vNormal / (mi + mk);

                                bodies[i].veloc[0] -= nNormal[0] * impulse * mk;
                                bodies[i].veloc[1] -= nNormal[1] * impulse * mk;
                                bodies[i].veloc[2] -= nNormal[2] * impulse * mk;

                                bodies[k].veloc[0] += nNormal[0] * impulse * mi;
                                bodies[k].veloc[1] += nNormal[1] * impulse * mi;
                                bodies[k].veloc[2] += nNormal[2] * impulse * mi;
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
    if (dimension == 2)
    {
        return (coords1[0] - coords2[0]) * (coords1[0] - coords2[0]) + (coords1[1] - coords2[1]) * (coords1[1] - coords2[1]) <= (r1 + r2) * (r1 + r2);
    }
    else
    {
        return (coords1[0] - coords2[0]) * (coords1[0] - coords2[0]) + (coords1[1] - coords2[1]) * (coords1[1] - coords2[1]) + (coords1[2] - coords2[2]) * (coords1[2] - coords2[2]) <= (r1 + r2) * (r1 + r2);
    }
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