#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "simulation/simulation.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
sim::States state = sim::States::MENU;

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

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
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
            break;
        case sim::States::ThreeBody2DSim:
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
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
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
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
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