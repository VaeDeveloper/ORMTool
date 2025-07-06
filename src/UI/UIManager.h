#pragma once 
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <GLFW/glfw3.h>

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
};

