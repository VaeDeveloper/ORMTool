#include "UIManager.h"

#include "UIManager.h"
#include <imgui.h>
#include <imgui_internal.h>

#include <nfd.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>
#include <filesystem>
#include <GLFW/glfw3.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <future>

#define STB_IMAGE_RESIZE_IMPLEMENTATION


#include <unordered_map>

namespace ImNeo 
{

	struct CheckboxState
	{
		ImVec4 background = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		ImVec4 text_colored = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImVec4 circle = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
		float circle_offset = 0.0f;
	};

	inline std::unordered_map<ImGuiID, CheckboxState> checkbox_states;

	bool Checkbox(const char* label, bool* value,
		float height = 22.0f,
		float toggle_width = 30.0f,
		float toggle_height = 14.0f,
		float radius = 5.0f,
		float spacing = 8.0f)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if(!window || window->SkipItems) return false;

		ImGuiID id = window->GetID(label);
		ImVec2 label_size = ImGui::CalcTextSize(label);
		float total_width = label_size.x + spacing + toggle_width;

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 toggle_pos = ImVec2(pos.x + label_size.x + spacing,
			pos.y + (height - toggle_height) * 0.5f);
		ImVec2 toggle_size = ImVec2(toggle_width, toggle_height);

		ImRect total_bb(pos, ImVec2(pos.x + total_width, pos.y + height));
		ImGui::ItemSize(total_bb);
		if(!ImGui::ItemAdd(total_bb, id)) return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

		auto& state = checkbox_states[id];

		ImGuiIO& io = ImGui::GetIO();
		ImVec4 target_text = *value ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) :
			hovered ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) :
			ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
		ImVec4 target_bg = *value ? ImVec4(0.2f, 0.6f, 0.3f, 1.0f) : ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		ImVec4 target_circle = *value ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

		state.text_colored = ImLerp(state.text_colored, target_text, io.DeltaTime * 8.0f);
		state.background = ImLerp(state.background, target_bg, io.DeltaTime * 8.0f);
		state.circle = ImLerp(state.circle, target_circle, io.DeltaTime * 8.0f);
		float left_offset = radius + 3.0f;
		float right_offset = toggle_width - radius - 3.0f;
		state.circle_offset = ImLerp(state.circle_offset, *value ? right_offset : left_offset, io.DeltaTime * 10.0f);

		if(pressed)
		{
			*value = !(*value);
			ImGui::MarkItemEdited(id);
		}

		// Draw toggle
		ImVec2 toggle_min = toggle_pos;
		ImVec2 toggle_max = ImVec2(toggle_min.x + toggle_width, toggle_min.y + toggle_height);
		ImVec2 circle_center = ImVec2(toggle_min.x + state.circle_offset, toggle_min.y + toggle_height * 0.5f);

		ImU32 bg_col = ImGui::ColorConvertFloat4ToU32(state.background);
		ImU32 circle_col = ImGui::ColorConvertFloat4ToU32(state.circle);
		ImU32 text_col = ImGui::ColorConvertFloat4ToU32(state.text_colored);

		window->DrawList->AddRectFilled(toggle_min, toggle_max, bg_col, toggle_height * 0.5f); // Rounded toggle
		window->DrawList->AddCircleFilled(circle_center, radius, circle_col, 12);             // Circle

		// Draw label
		window->DrawList->AddText(pos, text_col, label);

		// Fix SameLine
		ImGui::SetCursorScreenPos(ImVec2(pos.x + total_width, pos.y));
		return pressed;
	}


} // namespace my_widgets

namespace fs = std::filesystem;

UIManager::UIManager() {}

UIManager::~UIManager()
{

}

void UIManager::Initialize(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void UIManager::BeginFrame()
{
 
}

void UIManager::DrawUI()
{
	ShowMainUI();
	UpdatePreviewIfNeeded();
}

void UIManager::Render() 
{
}

void UIManager::Shutdown()
{
	aoPreview.Unload();
	roughPreview.Unload();
	metallicPreview.Unload();
	ormPreview.Unload();

}

bool PreviewTexture::Load(const std::string& p)
{
	Unload();
	path = p;
	int channels;
	data = stbi_load(p.c_str(), &width, &height, &channels, 3);
	if(!data) {
		std::cerr << "Failed to load image: " << p << std::endl;
		return false;
	}

	glGenTextures(1, &glId);
	glBindTexture(GL_TEXTURE_2D, glId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return true;
}

void PreviewTexture::Unload()
{
	if(glId) glDeleteTextures(1, &glId);
	if(channelR) glDeleteTextures(1, &channelR);
	if(channelG) glDeleteTextures(1, &channelG);
	if(channelB) glDeleteTextures(1, &channelB);
	if(data) stbi_image_free(data);
	glId = channelR = channelG = channelB = 0;
	data = nullptr;
}

void PreviewTexture::GenerateChannelsFromRGB(unsigned char* src, int w, int h) 
{
	width = w;
	height = h;

	std::vector<unsigned char> red(w * h);
	std::vector<unsigned char> green(w * h);
	std::vector<unsigned char> blue(w * h);

	for(int i = 0; i < w * h; ++i) {
		red[i] = src[i * 3 + 0];
		green[i] = src[i * 3 + 1];
		blue[i] = src[i * 3 + 2];
	}

	auto createTex = [] (GLuint& id, unsigned char* channelData, int w, int h) {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, channelData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		};

	createTex(channelR, red.data(), w, h);
	createTex(channelG, green.data(), w, h);
	createTex(channelB, blue.data(), w, h);
}

unsigned char* LoadGrayscale(const std::string& path, int& width, int& height) 
{
	int channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 1);
	if(!data) std::cerr << "Failed to load: " << path << "\n";
	return data;
}

bool UIManager::SaveUnrealAndUnityORM(
	const std::string& ao, const std::string& rough, const std::string& metal,
	const std::string& unrealPath, const std::string& unityPath,
	bool doUnreal, bool doUnity,
	const std::function<void(float)>& progressCallback)
{
	int w1, h1, w2, h2, w3, h3;
	unsigned char* aoData = LoadGrayscale(ao, w1, h1);
	unsigned char* roughData = LoadGrayscale(rough, w2, h2);
	unsigned char* metalData = LoadGrayscale(metal, w3, h3);

	if(!aoData || !roughData || !metalData) return false;
	if(w1 != w2 || w1 != w3 || h1 != h2 || h1 != h3) {
		std::cerr << "Size mismatch!\n";
		return false;
	}

	size_t count = w1 * h1;
	float totalSteps = 0.0f;
	if(doUnreal) totalSteps += 2.0f;
	if(doUnity) totalSteps += 2.0f;

	float currentStep = 0.0f;

	if(doUnreal) {
		std::vector<unsigned char> ormRGB(count * 3);
		for(size_t i = 0; i < count; ++i) {
			ormRGB[i * 3 + 0] = aoData[i];
			ormRGB[i * 3 + 1] = roughData[i];
			ormRGB[i * 3 + 2] = metalData[i];

			if(progressCallback && (i % (count / 100 + 1) == 0)) {
				float pixelProgress = static_cast<float>(i) / count;
				progressCallback((currentStep + pixelProgress) / totalSteps);
			}
		}
		currentStep += 1.0f;

		stbi_write_png(unrealPath.c_str(), w1, h1, 3, ormRGB.data(), w1 * 3);
		currentStep += 1.0f;
		if(progressCallback) progressCallback(currentStep / totalSteps);

		ormPreview.Unload();
		ormPreview.path = unrealPath;
		ormPreview.width = w1;
		ormPreview.height = h1;
		glGenTextures(1, &ormPreview.glId);
		glBindTexture(GL_TEXTURE_2D, ormPreview.glId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, ormRGB.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		ormPreview.GenerateChannelsFromRGB(ormRGB.data(), w1, h1);
	}

	if(doUnity) {
		std::vector<unsigned char> ormRGBA(count * 4);
		for(size_t i = 0; i < count; ++i) {
			ormRGBA[i * 4 + 0] = metalData[i];
			ormRGBA[i * 4 + 1] = aoData[i];
			ormRGBA[i * 4 + 2] = 255;
			ormRGBA[i * 4 + 3] = 255 - roughData[i];

			if(progressCallback && (i % (count / 100 + 1) == 0)) {
				float pixelProgress = static_cast<float>(i) / count;
				progressCallback((currentStep + pixelProgress) / totalSteps);
			}
		}
		currentStep += 1.0f;

		stbi_write_png(unityPath.c_str(), w1, h1, 4, ormRGBA.data(), w1 * 4);
		currentStep += 1.0f;
		if(progressCallback) progressCallback(currentStep / totalSteps);
	}

	stbi_image_free(aoData);
	stbi_image_free(roughData);
	stbi_image_free(metalData);

	if(progressCallback) progressCallback(1.0f);
	return true;
}

void UIManager::ShowMainUI()
{
	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{
			if(ImGui::BeginMenu("Save"))
			{
				if(ImGui::MenuItem("Save to PNG"))
				{
					std::cout << "Save selected\n";
				}
				if(ImGui::MenuItem("Save to JPG"))
				{
					std::cout << "Save selected\n";
				}
				ImGui::EndMenu();
			}

			if(ImGui::MenuItem("Exit"))
			{
				std::exit(0);
			}

			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("About"))
		{
			if(ImGui::MenuItem("About"))
			{
				std::cout << "Save selected\n";
			}

			if(ImGui::MenuItem("Git Link..."))
			{
				std::cout << "Save selected\n";
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}




	ImVec2 menuHeight = ImVec2(0, ImGui::GetFrameHeight());
	ImGui::SetNextWindowPos(menuHeight);
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - menuHeight.y));
	ImGui::Begin("ORMTool", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);


	ImGui::BeginChild("Header", ImVec2(686, 98), true);

	std::string generatedStringButton = generatingORM ? "Cancel" : "Generate ORM";
	if(ImGui::Button(generatedStringButton.c_str(), ImVec2(140, 30)) && !generatingORM) {
		generatingORM = true;
		ormProgress = 0.0f;

		loadingThread = std::thread([this] {
			SaveUnrealAndUnityORM(
				aoPreview.path, roughPreview.path, metallicPreview.path,
				generatedUnrealPath, outputUnity,
				generateUnrealORM, generateUnityORM,
				[this] (float p) { ormProgress = p; }
			);
			needsPreviewUpdate = true;
			generatingORM = false;
			});
		loadingThread.detach();
	}

	if(!generatingORM) ormProgress = 0.0f;



	ImGui::SameLine();
	ImGui::ProgressBar(ormProgress, ImVec2(522, 30), ormProgress == 0.0f ? "Push Start Generating " : "Generating...");
	// ImGui::Checkbox("Generate Unreal ORM (RGB)", &generateUnrealORM);
	ImGui::Dummy(ImVec2(0.0f, 2.0f));
	ImNeo::Checkbox("Unreal ORM (RGB)", &generateUnrealORM, 14.0f);
	ImGui::SameLine();
	ImNeo::Checkbox("Unity (RGBA)", &generateUnityORM, 14.0f);
	ImGui::SameLine(400.f, 2.0f);
	
	// ImGui::Dummy(ImVec2(4.0f, 0.0f));
	ImGui::SetNextItemWidth(150.0f);
	ImGui::Combo("Channel", (int*)&selectedChannel, "All (RGB)\0AO (R)\0Roughness (G)\0Metallic (B)\0");

	ImGui::EndChild();
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 145);

	auto showTextureBlock = [&] (const char* label, PreviewTexture& tex, const char* title, int& resolutionIndex, ImVec4 borderColor) {
		ImGui::PushID(label);
		ImGui::TextUnformatted(title);
		ImGui::SetNextItemWidth(135.0f);
		ImGui::Combo("##resCombo", &resolutionIndex, resolutionOptions, IM_ARRAYSIZE(resolutionOptions));

		ImGui::PushStyleColor(ImGuiCol_Button, borderColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

		if(ImGui::ImageButton(label, (ImTextureID)(intptr_t)tex.glId, ImVec2(128, 128))) {
			nfdchar_t* outPath = nullptr;
			if(NFD_OpenDialog("png,jpg", nullptr, &outPath) == NFD_OKAY) {
				tex.Unload();
				tex.path = outPath;
				if(tex.Load(outPath)) {
					for(int i = 0; i < IM_ARRAYSIZE(resolutionValues); ++i) {
						if(tex.width == resolutionValues[i]) {
							resolutionIndex = i;
							break;
						}
					}
				}
				free(outPath);
			}
		}

		ImGui::PopStyleColor(3);
		ImGui::Dummy(ImVec2(0, 3));
		ImGui::PopID();
		};

	showTextureBlock("AO", aoPreview, "AO", aoResolutionIndex, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	showTextureBlock("Rough", roughPreview, "Roughness", roughResolutionIndex, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
	showTextureBlock("Metal", metallicPreview, "Metallic", metalResolutionIndex, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));

	ImGui::NextColumn();
	ImGui::BeginChild("Viewport", ImVec2(0, 544), true);
	float previewWidth = ImGui::GetContentRegionAvail().x - 1.0f;
	float aspect = ormPreview.width > 0 ? (float)ormPreview.height / ormPreview.width : 1.0f;
	float previewHeight = previewWidth * aspect;

	GLuint texId = ormPreview.glId;
	if(selectedChannel == ORMChannel::AO_R) texId = ormPreview.channelR;
	else if(selectedChannel == ORMChannel::Roughness_G) texId = ormPreview.channelG;
	else if(selectedChannel == ORMChannel::Metallic_B) texId = ormPreview.channelB;

	if(texId)
	{
		ImGui::Image((ImTextureID)(intptr_t)texId, ImVec2(previewWidth, previewHeight));
	}
	else
	{
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 loaderPos = ImVec2(
			pos.x + previewWidth - 85,
			pos.y + previewHeight - 90
		);
		if (generatingORM )
			AddLoadingCube("Generate", loaderPos);

		if(loadingTexture)
			AddLoadingCube("Loading", loaderPos);
	}

	ImGui::EndChild();
	ImGui::Columns(1);


	ImGui::End();


}

void UIManager::UpdatePreviewIfNeeded() {
	if(!needsPreviewUpdate) return;

	ormPreview.Unload();
	ormPreview.path = generatedUnrealPath;

	int w, h, channels;
	unsigned char* data = stbi_load(generatedUnrealPath.c_str(), &w, &h, &channels, 3);
	if(data) {
		ormPreview.width = w;
		ormPreview.height = h;
		glGenTextures(1, &ormPreview.glId);
		glBindTexture(GL_TEXTURE_2D, ormPreview.glId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		ormPreview.GenerateChannelsFromRGB(data, w, h);
		stbi_image_free(data);
	}

	needsPreviewUpdate = false;
}