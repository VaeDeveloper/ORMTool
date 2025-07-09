#include "App.h"

#include <iostream>
#include "Constants.h"




namespace ImNeo
{
	void custom_cursor(ImVec2 position, float rotation_angle_degrees, ImColor color) {
		float arrow_base_width = 8;
		float arrow_height = 9;
		float arrow_middle_width = 7;
		float outline_thickness = 2.0f;


		ImVec2 verts[4];
		verts[0] = position;
		verts[1] = ImVec2(position.x - arrow_base_width / 2, position.y + arrow_height);
		verts[2] = ImVec2(position.x, position.y + arrow_middle_width);
		verts[3] = ImVec2(position.x + arrow_base_width / 2, position.y + arrow_height);


		ImVec2 center = ImVec2(position.x, position.y + arrow_height / 2);

		float angle_radians = rotation_angle_degrees * (3.14159f / 180.0f);

		auto rotate_vertex = [&] (ImVec2 point, ImVec2 center, float angle) -> ImVec2 {
			float translated_x = point.x - center.x;
			float translated_y = point.y - center.y;

			float rotated_x = translated_x * cos(angle) - translated_y * sin(angle);
			float rotated_y = translated_x * sin(angle) + translated_y * cos(angle);

			return ImVec2(rotated_x + center.x, rotated_y + center.y);
			};

		verts[0] = rotate_vertex(verts[0], center, angle_radians);
		verts[1] = rotate_vertex(verts[1], center, angle_radians);
		verts[2] = rotate_vertex(verts[2], center, angle_radians);
		verts[3] = rotate_vertex(verts[3], center, angle_radians);

		ImDrawList* draw_list = ImGui::GetForegroundDrawList();

		draw_list->AddPolyline(verts, 4, color, true, outline_thickness);

		draw_list->AddConvexPolyFilled(verts, 4, color);
	}
}


Application::Application() : uiManager(std::make_unique<UIManager>())
{
	
}

Application::~Application()
{
	Shutdown();
}

[[nodiscard]] InitStatus Application::InitializeApplication()
{
	if(initStatus.has_value())
		return *initStatus;

	const auto glfwStatus = InitializeGLFW();
	if(glfwStatus != InitStatus::Success) {
		std::cerr << GetInitStatus(glfwStatus) << " at " << __FUNCTION__ << ":" << __LINE__ << "\n";
		initStatus = glfwStatus;
		return glfwStatus;
	}
	uiManager->Initialize(window);

	//// ImGui init block
	int major, minor, rev;
	glfwGetVersion(&major, &minor, &rev);
	std::cout << "GLFW Version: " << major << "." << minor << "." << rev << "\n";
	std::cout << "GLFW Initialized successfully\n";

	initStatus = InitStatus::Success;
	return InitStatus::Success;
}

void Application::RunApplication()
{
	if(!IsInitialized()) {
		std::cerr << "Application not initialized!\n";
		return;
	}
	std::cout << "Cursor hidden status: " << glfwGetInputMode(window, GLFW_CURSOR) << std::endl;
	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		RenderScene();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		uiManager->BeginFrame();

		uiManager->DrawUI();

		ImGui::Begin("##InvisibleCursorWindow", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoBringToFrontOnFocus);

		ImVec2 mouse_pos = ImGui::GetIO().MousePos;
		ImNeo::custom_cursor(mouse_pos, -45.0f, ImColor(255, 255, 255));

		ImGui::End();

		uiManager->Render();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
}

void Application::Shutdown()
{
	if(uiManager)
	{
		uiManager->Shutdown();
		uiManager.reset();
	}
	
	if(window) 
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}

	// uiManager->Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	initStatus.reset();
}
bool Application::IsInitialized() const
{
	return initStatus.has_value() && *initStatus == InitStatus::Success;
}

std::string_view Application::GetInitStatus(InitStatus status) 
{
	switch(status) 
	{
	case InitStatus::Success: return "Success";
	case InitStatus::GLFW_InitFailed: return "GLFW initialization failed";
	case InitStatus::WindowCreationFailed: return "Window creation failed";
	case InitStatus::OpenGL_InitFailed: return "OpenGL initialization failed";
	case InitStatus::Fail: return "Generic failure";
	default: return "Unknown error";
	}
}

[[nodiscard]] InitStatus Application::InitializeGLFW() 
{
	glfwSetErrorCallback(GLFWErrorCallback);

	if(!glfwInit()) 
	{
		return InitStatus::GLFW_InitFailed;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(ORM::WindowWidth, ORM::WindowHeight, ORM::TitleStr,nullptr, nullptr);
	if(!window) 
	{
		return InitStatus::WindowCreationFailed;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	return InitStatus::Success;
}



void Application::RenderScene()
{
	glViewport(0, 0, ORM::WindowWidth, ORM::WindowHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // ярко-розовый
	glClear(GL_COLOR_BUFFER_BIT);
}

void Application::GLFWErrorCallback(int error, const char* description)
{
	std::cerr << "[GLFW Error] (" << error << "): " << description << std::endl;
}



