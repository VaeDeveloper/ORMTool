#pragma once 
#include <string>
#include <atomic>
#include <future>
#include <thread>
#include <mutex>
#include <functional>
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <map>


// Операции с ImVec2
inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
	return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) {
	return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline ImVec2 operator*(const ImVec2& lhs, float rhs) {
	return ImVec2(lhs.x * rhs, lhs.y * rhs);
}

inline ImVec2 operator*(float lhs, const ImVec2& rhs) {
	return ImVec2(lhs * rhs.x, lhs * rhs.y);
}

inline ImVec2 operator/(const ImVec2& lhs, float rhs) {
	return ImVec2(lhs.x / rhs, lhs.y / rhs);
}

struct ImVec3 {
	float x, y, z;
	ImVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

inline ImVec3 RotateX(const ImVec3& v, float angle) {
	float rad = angle * (3.14159f / 180.0f);
	float cosA = cos(rad);
	float sinA = sin(rad);
	return ImVec3(v.x, v.y * cosA - v.z * sinA, v.y * sinA + v.z * cosA);
}

inline ImVec3 RotateY(const ImVec3& v, float angle) {
	float rad = angle * (3.14159f / 180.0f);
	float cosA = cos(rad);
	float sinA = sin(rad);
	return ImVec3(v.x * cosA + v.z * sinA, v.y, -v.x * sinA + v.z * cosA);
}

inline void Add3DCube(ImVec2 center, float size, float perspective, float rotationX, float rotationY, float distance)
{
	std::array<ImVec3, 8> cube_vertices{ {
		ImVec3(-1, -1, -1), ImVec3(1, -1, -1),
		ImVec3(1,  1, -1), ImVec3(-1,  1, -1),
		ImVec3(-1, -1,  1), ImVec3(1, -1,  1),
		ImVec3(1,  1,  1), ImVec3(-1,  1,  1)
	} };

	std::array<ImVec2, 8> projected_vertices;

	for(size_t i = 0; i < cube_vertices.size(); i++)
	{
		ImVec3 rotated = RotateX(cube_vertices[i], rotationX);
		rotated = RotateY(rotated, rotationY);

		rotated.x *= distance;
		rotated.y *= distance;
		rotated.z *= distance;

		float z = rotated.z * perspective + 3.0f;
		projected_vertices[i] = ImVec2(
			center.x + rotated.x * size / z,
			center.y + rotated.y * size / z
		);

		//ImGui::GetWindowDrawList()->AddCircle(
		//    projected_vertices[i], 3.5f, ImColor(255, 255, 0, 255), 24, 2.0f
		//);
	}

	std::array<std::pair<int, int>, 12> edges = { {
		{0, 1}, {1, 2}, {2, 3}, {3, 0},
		{4, 5}, {5, 6}, {6, 7}, {7, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	} };

	for(const auto& edge : edges)
	{
		ImGui::GetWindowDrawList()->AddLine(
			projected_vertices[edge.first],
			projected_vertices[edge.second],
			IM_COL32(255, 255, 255, 255), 2.0f
		);
	}
}

struct AnimationState {
	bool state;
	float target_speed;
	float speed_multiplier;
	float rotation_x;
	float rotation_y;
};

inline void AddLoadingCube(const char* label, ImVec2 position)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiID id = window->GetID(label);
	if(window->SkipItems)
		return;

	ImGui::PushID(id);
	auto& g = *GImGui;
	auto delta_time = ImGui::GetIO().DeltaTime;

	ImVec2 text_size = ImGui::CalcTextSize(label);
	const float spacing = 25.f;
	const float cube_size = 50.f;
	const ImVec2 extra_bb = ImVec2(cube_size + spacing, cube_size + spacing);

	const ImRect bb(position, position + extra_bb);
	ImGui::ItemSize(extra_bb);
	ImGui::ItemAdd(bb, id);

	static std::map<ImGuiID, AnimationState> animations;
	if(animations.find(id) == animations.end())
		animations[id] = AnimationState{ true, 68.0f, 0.0f, 0.0f, 0.0f };

	AnimationState& anim_state = animations[id];
	float lerp_factor = delta_time * 3.0f;

	if(anim_state.state) {
		anim_state.speed_multiplier = ImLerp(anim_state.speed_multiplier, anim_state.target_speed, lerp_factor);
		if(anim_state.speed_multiplier >= anim_state.target_speed * 0.95f)
			anim_state.state = false;
	}
	else {
		anim_state.speed_multiplier = ImLerp(anim_state.speed_multiplier, 2.0f, lerp_factor);
		if(anim_state.speed_multiplier <= 2.4f)
			anim_state.state = true;
	}

	anim_state.rotation_x += delta_time * (8.5f * anim_state.speed_multiplier);
	anim_state.rotation_y -= delta_time * (8.0f * anim_state.speed_multiplier);
	float scale_factor = 1.0f + (anim_state.speed_multiplier * 0.0012f);

	Add3DCube(
		position + ImVec2(cube_size / 2.f + spacing / 2.f, cube_size / 2.f + spacing / 4.f),
		cube_size,
		0.6f,
		anim_state.rotation_x,
		anim_state.rotation_y,
		scale_factor
	);

	if(ImGui::GetIO().Fonts->Fonts.Size > 8 && ImGui::GetIO().Fonts->Fonts[8])
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[8]);
	else
		ImGui::PushFont(ImGui::GetFont());

	static std::map<ImGuiID, float> values;
	float& val = values[id];
	val = ImLerp(val, 200.f, g.IO.DeltaTime * 10.f);

	ImVec2 text_pos = ImVec2(
		bb.Min.x + (bb.GetWidth() - text_size.x) / 2.0f,
		bb.Min.y + bb.GetHeight() + 5.0f
	);

	ImGui::GetWindowDrawList()->AddText(text_pos, ImColor(255, 255, 255, int((anim_state.speed_multiplier * 2) + 135)), label);

	ImGui::PopFont();
	ImGui::PopID();
}

struct PreviewTexture
{
	std::string path;
	GLuint glId = 0;
	GLuint channelR = 0, channelG = 0, channelB = 0;
	int width = 0, height = 0;
	unsigned char* data = nullptr;

	bool Load(const std::string& p);
	void Unload();
	void GenerateChannelsFromRGB(unsigned char* src, int w, int h);
};

enum class ORMChannel { AllRGB, AO_R, Roughness_G, Metallic_B };

class UIManager
{
public:
	UIManager();
	~UIManager();

	void Initialize(GLFWwindow* window);
	void BeginFrame();
	void DrawUI();
	void Render();
	void Shutdown();

private:

	// UI state and logic
	void ShowMainUI();
	void UpdatePreviewIfNeeded();

	// Image loading/generation
	bool SaveUnrealAndUnityORM(
		const std::string& ao, const std::string& rough, const std::string& metal,
		const std::string& unrealPath, const std::string& unityPath,
		bool doUnreal, bool doUnity,
		const std::function<void(float)>& progressCallback = nullptr
	);

	// Internal state
	PreviewTexture aoPreview, roughPreview, metallicPreview, ormPreview;

	bool generateUnrealORM = true;
	bool generateUnityORM = true;
	ORMChannel selectedChannel = ORMChannel::AllRGB;

	int aoResolutionIndex = 0;
	int roughResolutionIndex = 0;
	int metalResolutionIndex = 0;

	const char* resolutionOptions[6] = { "128","256","512","1024","2048","4096" };
	const int resolutionValues[6] = { 128, 256, 512, 1024, 2048, 4096 };

	std::atomic<bool> needsPreviewUpdate = false;
	std::atomic<bool> generatingORM = false;
	float ormProgress = 0.0f;
	std::string generatedUnrealPath = "orm_unreal.png";
	std::string outputUnity = "orm_unity.png";

	std::mutex loadingMutex;
	std::thread loadingThread;

	std::atomic<bool> loadingTexture;
};

