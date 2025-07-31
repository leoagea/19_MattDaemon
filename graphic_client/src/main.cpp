/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 10:52:41 by lagea             #+#    #+#             */
/*   Updated: 2025/07/31 12:21:43 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl2.h"

#include <GL/glew.h>          // or glad.h
#include <GLFW/glfw3.h>
#include <cstdio>

#include "../inc/graphic_client.h"

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
	bool show_demo = false;
	bool show_log = false;
	bool show_terminal = false;
	bool daemon_running = isDaemonRunning();
	static char input_buffer[256] = "";

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		/* ---- Your widgets here ---- */
		if (show_demo) ImGui::ShowDemoWindow(&show_demo);

		ImGui::Begin("Matt Daemon Panel");
		
		// Daemon control buttons
		if (ImGui::Button("Start Daemon") && !daemon_running) {
			std::string command = std::string(DAEMON_EXEC_PATH) + " &";
			if (system(command.c_str()) == 0)
				daemon_running = true;
			else
				std::cerr << "Failed to start daemon\n";
		}
		
		ImGui::SameLine();
		std::string daemon_status = daemon_running ? "Running" : "Not Running";
		ImGui::Text("| %s", daemon_status.c_str());

		if (ImGui::Button("Kill Daemon") && daemon_running)
			if (killDaemon() == SUCCESS)
				daemon_running = false;

		std::string status = getDaemonStatus();
		if (!status.empty()) ImGui::Text("%s", status.c_str());

		// Connection status
		if (ImGui::Button(isConnected() ? "Disconnect" : "Connect")) {
			if (isConnected())
				disconnectFromDaemon();
			else
				connectToDaemon();
		}

		ImGui::SameLine();
		std::string conn_status = getConnectionStatus();
		if (!conn_status.empty())
			ImGui::Text("| %s", conn_status.c_str());
		
		ImGui::Separator();

		// Terminal Section
		if (ImGui::Button("Show Terminal")) show_terminal = !show_terminal;

		if (show_terminal) {
			ImGui::Begin("Daemon Terminal", &show_terminal);

			ImGui::Text("Send command to daemon:");
			if (ImGui::InputText("##input", input_buffer, sizeof(input_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
				if (isConnected() && strlen(input_buffer) > 0)
					if (sendMessage(std::string(input_buffer), daemon_running))
						input_buffer[0] = '\0';

			ImGui::SameLine();
			if (ImGui::Button("Send") && isConnected() && strlen(input_buffer) > 0)
				if (sendMessage(std::string(input_buffer), daemon_running))
					input_buffer[0] = '\0';

			// Connection status in terminal window
			if (!isConnected())
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not connected to daemon");
			else
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected - Type commands above");
			
			ImGui::End();
		}

		// Log Buttons
		if (ImGui::Button("Clear Log")) clearLog();
		ImGui::SameLine();
		if (ImGui::Button("Show Log")) show_log = !show_log;
		if (show_log){
			ImGui::BeginChild("Log", ImVec2(0, 200), true);
			ImGui::TextWrapped("%s", getLogContent().c_str());
			ImGui::EndChild();
		}
		
		// ImGui::Checkbox("Show Demo Window", &show_demo);
		if (ImGui::Button("Quit Window")) glfwSetWindowShouldClose(window, true);

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
