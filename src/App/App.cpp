#include "App.h"

#include <iostream>
#include "Constants.h"






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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // ����-�������
	glClear(GL_COLOR_BUFFER_BIT);
}

void Application::GLFWErrorCallback(int error, const char* description)
{
	std::cerr << "[GLFW Error] (" << error << "): " << description << std::endl;
}



