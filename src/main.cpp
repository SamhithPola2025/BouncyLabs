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

    std::cout << glGetString(GL_VERSION) << std::endl;

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

        static bool gravityEnabled = true;
        ImGui::Checkbox("Enable Gravity", &particleSystem.gravityEnabled);

        if (gravityEnabled) {
            for (auto& p : particleSystem.points) {
                p.ay = -9.8f;
            }
            for (auto& square : particleSystem.squares) {
                for (auto* point : {&square.point1, &square.point2, &square.point3, &square.point4}) {
                    point->ay = -9.8f;
                }
            }
        } else {
            for (auto& p : particleSystem.points) {
                p.ay = 0.0f;
            }
            for (auto& square : particleSystem.squares) {
                for (auto* point : {&square.point1, &square.point2, &square.point3, &square.point4}) {
                    point->ay = 0.0f;
                }
            }
        }

        if (ImGui::Button("Create Particle")) {
            create_particle(x, y, radius, vx, vy, ax, ay);
        }

        if (ImGui::Button("Delete Selected Particle")) {
            for (auto it = particleSystem.points.begin(); it != particleSystem.points.end();) {
                float dx = ImGui::GetMousePos().x - it->x;
                float dy = ImGui::GetMousePos().y - it->y;
                if (std::sqrt(dx * dx + dy * dy) < it->radius + 5.0f) {
                    it = particleSystem.points.erase(it); 
                } else {
                    ++it;
                }
            }
        }

        ImGui::End();

        static float triangleX = 200.0f, triangleY = 200.0f;
        static float triangleSideLength = 50.0f;
        static float triangleVX = 0.0f, triangleVY = 0.0f;

        ImGui::Begin("Triangle Controls");
        ImGui::SliderFloat("X Position", &triangleX, 0.0f, 1280.0f);
        ImGui::SliderFloat("Y Position", &triangleY, 0.0f, 720.0f);
        ImGui::SliderFloat("Side Length", &triangleSideLength, 10.0f, 200.0f);
        ImGui::SliderFloat("X Velocity", &triangleVX, -100.0f, 100.0f);
        ImGui::SliderFloat("Y Velocity", &triangleVY, -100.0f, 100.0f);

        if (ImGui::Button("Create Triangle")) {
            Triangle triangle;

            triangle.point1 = {triangleX, triangleY, 5.0f, triangleVX, triangleVY, 0.0f, 0.0f};
            triangle.point2 = {triangleX + triangleSideLength, triangleY, 5.0f, triangleVX, triangleVY, 0.0f, 0.0f};
            triangle.point3 = {triangleX + triangleSideLength / 2.0f, triangleY + triangleSideLength * std::sqrt(3.0f) / 2.0f, 5.0f, triangleVX, triangleVY, 0.0f, 0.0f};

            particleSystem.addTriangle(triangle);
        }
        ImGui::End();

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

            square.point1 = {squareX, squareY, 5.0f, squareVX, squareVY, 0.0f, 0.0f};
            square.point2 = {squareX + squareSideLength, squareY, 5.0f, squareVX, squareVY, 0.0f, 0.0f};
            square.point3 = {squareX + squareSideLength, squareY + squareSideLength, 5.0f, squareVX, squareVY, 0.0f, 0.0f};
            square.point4 = {squareX, squareY + squareSideLength, 5.0f, squareVX, squareVY, 0.0f, 0.0f};

            particleSystem.squares.push_back(square);
        }

        if (ImGui::Button("Delete Selected Square")) {
            ImVec2 mousePos = ImGui::GetMousePos(); 
            for (auto it = particleSystem.squares.begin(); it != particleSystem.squares.end(); ++it) {
                float minX = std::min({it->point1.x, it->point2.x, it->point3.x, it->point4.x});
                float maxX = std::max({it->point1.x, it->point2.x, it->point3.x, it->point4.x});
                float minY = std::min({it->point1.y, it->point2.y, it->point3.y, it->point4.y});
                float maxY = std::max({it->point1.y, it->point2.y, it->point3.y, it->point4.y});
        
                if (mousePos.x >= minX && mousePos.x <= maxX && mousePos.y >= minY && mousePos.y <= maxY) {
                    particleSystem.squares.erase(it); 
                    break; 
                }
            }
        }

        ImGui::End();

        ImGui::Begin("External features");
        static float gravityStrength = 9.8f;
        static float windStrength = 0.0f;
        ImGui::SliderFloat("Gravity Strength", &gravityStrength, -60.0f, 180.0f);
        ImGui::SliderFloat("Wind Strength", &windStrength,  -50.0f, 50.0f);

        for (auto& p : particleSystem.points) {
            p.ax =windStrength;
        }
        for (auto& t : particleSystem.triangles) {
            for (auto* pt : {&t.point1, &t.point2, &t.point3}) {
                pt->ax = windStrength;
            }
        }
        for (auto& s : particleSystem.squares) {
            for (auto* pt : {&s.point1, &s.point2, &s.point3, &s.point4}) {
                pt->ax = windStrength;
            }
        }
        if (gravityEnabled) {
            for (auto& p : particleSystem.points) {
                p.ay = -gravityStrength;
            }
            for (auto& square : particleSystem.squares) {
                for (auto* point : {&square.point1, &square.point2, &square.point3, &square.point4}) {
                    point->ay = -gravityStrength;
                }
            }
        }

        ImGui::End();

        particleSystem.update(DELTATIME, gravityStrength);

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

        for (const auto& triangle : particleSystem.triangles) {
            auto* drawList = ImGui::GetForegroundDrawList();

            drawList->AddLine(ImVec2(triangle.point1.x, triangle.point1.y),
            ImVec2(triangle.point2.x, triangle.point2.y),
            IM_COL32(255, 255, 255, 255), 2.0f);

            drawList->AddLine(ImVec2(triangle.point2.x, triangle.point2.y),
            ImVec2(triangle.point3.x, triangle.point3.y),
            IM_COL32(255, 255, 255, 255), 2.0f);

            drawList->AddLine(ImVec2(triangle.point3.x, triangle.point3.y),
            ImVec2(triangle.point1.x, triangle.point1.y),
            IM_COL32(255, 255, 255, 255), 2.0f);
        } 

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

        for (auto& square : particleSystem.squares) {
            for (auto* point : {&square.point1, &square.point2, &square.point3, &square.point4}) {
                ImVec2 mousePos = ImGui::GetMousePos();
                float dx = mousePos.x - point->x;
                float dy = mousePos.y - point->y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < point->radius + 5.0f) {
                    ImGui::GetForegroundDrawList()->AddCircle(
                        ImVec2(point->x, point->y), point->radius + 3.0f, IM_COL32(0, 255, 0, 255), 12, 2.0f);
        
                    if (ImGui::IsMouseDown(0)) {
                        if (!point->dragged) {
                            point->dragged = true;
                            point->vx = 0.0f;
                            point->vy = 0.0f;
                            point->ax = 0.0f;
                            point->ay = 0.0f;
                            point->offsetX = mousePos.x - point->x;
                            point->offsetY = mousePos.y - point->y;
                        }
        
                        point->x = mousePos.x - point->offsetX;
                        point->y = mousePos.y - point->offsetY;
                    } else {
                        point->dragged = false;
                    }
                }
            }
        }

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

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