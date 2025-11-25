#include "structures.hpp"
#include "../dependencies/imgui/backends/imgui.h"
#include "../dependencies/imgui/backends/imgui_impl_glfw.h"
#include "../dependencies/glad/include/glad/glad.h"
#include "../dependencies/imgui/backends/imgui_impl_opengl3.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <GLFW/glfw3.h>

void create_particle(
    float x,
    float y, 
    float radius,
    float vx,
    float vy,
    float ax,
    float ay,
    float mass = 1.0f,
    float restitution = 0.8f,
    float friction = 0,
    bool fixed = false,
    float damping = 0.99f
);

// Global variables
const float DELTATIME = 0.016f;
ParticleSystem particleSystem;

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "BouncyLabs", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    std::cout << glGetString(GL_VERSION) << std::endl;  // works after GL context

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120");

    float bgColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    
        // Background color controls
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::Begin("Background Color", nullptr, ImGuiWindowFlags_NoMove);
        ImGui::Text("Adjust the background color:");
        ImGui::ColorEdit4("Background Color", bgColor); 
        ImGui::End();

        // Particle controls
        static float x = 200.0f, y = 200.0f;
        static float radius = 10.0f;
        static float vx = 0.0f, vy = 0.0f;
        static float ax = 0.0f, ay = -9.8f;

        ImGui::Begin("Particle Controls");
        ImGui::SliderFloat("X Position", &x, 0.0f, 1280.0f);
        ImGui::SliderFloat("Y Position", &y, 0.0f, 720.0f);
        ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f);
        ImGui::SliderFloat("X Velocity", &vx, -100.0f, 100.0f);
        ImGui::SliderFloat("Y Velocity", &vy, -100.0f, 100.0f);
        ImGui::SliderFloat("X Acceleration", &ax, -10.0f, 10.0f);
        ImGui::SliderFloat("Y Acceleration", &ay, -10.0f, 10.0f);

        if (ImGui::Button("Create Particle")) {
            create_particle(x, y, radius, vx, vy, ax, ay);
        }
        ImGui::End();

        // Square controls
        static float squareX = 100.0f, squareY = 100.0f;
        static float squareSideLength = 50.0f;
        static float squareVX = 0.0f, squareVY = 0.0f;

        ImGui::Begin("Square Controls");
        ImGui::SliderFloat("X Position", &squareX, 0.0f, 1280.0f);
        ImGui::SliderFloat("Y Position", &squareY, 0.0f, 720.0f);
        ImGui::SliderFloat("Side Length", &squareSideLength, 10.0f, 200.0f);
        ImGui::SliderFloat("X Velocity", &squareVX, -100.0f, 100.0f);
        ImGui::SliderFloat("Y Velocity", &squareVY, -100.0f, 100.0f);

        if (ImGui::Button("Create Square")) {
            Square square;
            square.sideLength = squareSideLength;

            // Initialize the square's points
            square.point1 = {squareX, squareY, 5.0f, squareVX, squareVY, 0.0f, 0.0f};
            square.point2 = {squareX + squareSideLength, squareY, 5.0f, squareVX, squareVY, 0.0f, 0.0f};
            square.point3 = {squareX + squareSideLength, squareY + squareSideLength, 5.0f, squareVX, squareVY, 0.0f, 0.0f};
            square.point4 = {squareX, squareY + squareSideLength, 5.0f, squareVX, squareVY, 0.0f, 0.0f};

            particleSystem.squares.push_back(square);
        }
        ImGui::End();

        // Update the particle system
        particleSystem.update(DELTATIME);

        // Render particles
        for (const auto& p : particleSystem.points) {
            ImGui::GetForegroundDrawList()->AddCircleFilled(
                ImVec2(p.x, p.y), p.radius, IM_COL32(255, 0, 0, 255));
        }

        for (auto& square : particleSystem.squares) {
            for (auto& point : {&square.point1, &square.point2, &square.point3, &square.point4}) {
                ImVec2 mousePos = ImGui::GetMousePos();
                float dx = mousePos.x - point->x;
                float dy = mousePos.y - point->y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < point->radius + 5.0f) {
                    ImGui::GetForegroundDrawList()->AddCircle(
                        ImVec2(point->x, point->y), point->radius + 3.0f, IM_COL32(0, 255, 0, 255), 12, 2.0f);
                    
                }
            }
        }

        // Render squares
        for (const auto& square : particleSystem.squares) {
            auto* drawList = ImGui::GetForegroundDrawList();

            drawList->AddLine(ImVec2(square.point1.x, square.point1.y),
                              ImVec2(square.point2.x, square.point2.y),
                              IM_COL32(255, 255, 255, 255), 2.0f);

            drawList->AddLine(ImVec2(square.point2.x, square.point2.y),
                              ImVec2(square.point3.x, square.point3.y),
                              IM_COL32(255, 255, 255, 255), 2.0f);

            drawList->AddLine(ImVec2(square.point3.x, square.point3.y),
                              ImVec2(square.point4.x, square.point4.y),
                              IM_COL32(255, 255, 255, 255), 2.0f);

            drawList->AddLine(ImVec2(square.point4.x, square.point4.y),
                              ImVec2(square.point1.x, square.point1.y),
                              IM_COL32(255, 255, 255, 255), 2.0f);
        }

        // Render ImGui
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void create_particle(    
    float x,
    float y,
    float radius,
    float vx,
    float vy,
    float ax, 
    float ay,
    float mass,
    float restitution,
    float friction,
    bool fixed,
    float damping
) {
    Point newParticle = {x, y, radius, vx, vy, ax, ay, mass, restitution, friction, fixed, damping};
    particleSystem.add(newParticle); 
}