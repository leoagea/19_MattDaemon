// ------------- src/main.cpp ----------------
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl2.h"

#include <GL/glew.h>          // or glad.h
#include <GLFW/glfw3.h>
#include <cstdio>

int main()
{
    /* 1. GLFW init */
    if (!glfwInit()) { std::fprintf(stderr, "GLFW init failed\n"); return 1; }
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Matt Daemon Client", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);          // Enable vsync

    /* 2. GLEW / GLAD */
    if (glewInit() != GLEW_OK) { std::fprintf(stderr, "GLEW init failed\n"); return 1; }

    /* 3. ImGui core + back-ends */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();
    ImGui::StyleColorsDark();     // or StyleColorsClassic()

    /* 4. Main loop */
    bool show_demo = true;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /* ---- Your widgets here ---- */
        if (show_demo) ImGui::ShowDemoWindow(&show_demo);
        ImGui::Begin("Matt Daemon Panel");
        ImGui::Text("Connected: false");
        ImGui::Button("Connect"); ImGui::SameLine();
        ImGui::Button("Quit daemon");
        ImGui::End();
        /* --------------------------- */

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    /* 5. Cleanup */
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
