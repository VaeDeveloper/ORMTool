

#include <iostream>
#include "App.h"


int main()
{
    Application app;

    if(app.InitializeApplication() != InitStatus::Success)
    {
        std::cerr << "Failed to initialize Application\n";
        return -1;
    }

    app.RunApplication();

    return 0;
}


#pragma region PrototyCode
//namespace fs = std::filesystem;
//
//// Globals
//bool generateUnrealORM = true;
//bool generateUnityORM = true;
//
//float ormProgress = 0.0f;
//bool generatingORM = false;
//float ormStartTime = 0.0f;
//
//std::atomic<bool> needsPreviewUpdate = false;
//std::string generatedUnrealPath;
//
//std::atomic<bool> isLoading(false);
//std::atomic<float> loadProgress(0.0f);
//std::string currentLoadingLabel;
//std::thread loadingThread;
//std::mutex loadingMutex;
//
//std::string outputUnreal = "orm_unreal.png";
//std::string outputUnity = "orm_unity.png";
//
//enum class ORMChannel { AllRGB, AO_R, Roughness_G, Metallic_B };
//ORMChannel selectedChannel = ORMChannel::AllRGB;
//
//int aoResolutionIndex = 0;
//int roughResolutionIndex = 0;
//int metalResolutionIndex = 0;
//
//const char* resolutionOptions[] = { "128","256", "512", "1024", "2048", "4096" };
//const int resolutionValues[] = { 128, 256, 512, 1024, 2048, 4096 };
//const int resolutionCount = IM_ARRAYSIZE(resolutionOptions);
//
//unsigned char* LoadGrayscale(const std::string& path, int& width, int& height) {
//    int channels;
//    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 1);
//    if(!data) std::cerr << "Failed to load: " << path << "\n";
//    return data;
//}
//
//struct PreviewTexture {
//    std::string path;
//    GLuint glId = 0;
//    GLuint channelR = 0, channelG = 0, channelB = 0;
//    int width = 0, height = 0;
//    unsigned char* data = nullptr;
//
//    void Unload() {
//        if(glId) glDeleteTextures(1, &glId);
//        if(channelR) glDeleteTextures(1, &channelR);
//        if(channelG) glDeleteTextures(1, &channelG);
//        if(channelB) glDeleteTextures(1, &channelB);
//        if(data) stbi_image_free(data);
//        glId = channelR = channelG = channelB = 0;
//        data = nullptr;
//    }
//
//    bool Load(const std::string& p) {
//        Unload();
//        path = p;
//        int channels;
//        data = stbi_load(p.c_str(), &width, &height, &channels, 3);
//        if(!data) {
//            std::cerr << "Failed to load image: " << p << std::endl;
//            return false;
//        }
//
//        glGenTextures(1, &glId);
//        glBindTexture(GL_TEXTURE_2D, glId);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        return true;
//    }
//
//    void GenerateChannelsFromRGB(unsigned char* src, int w, int h) {
//        width = w;
//        height = h;
//
//        std::vector<unsigned char> red(w * h);
//        std::vector<unsigned char> green(w * h);
//        std::vector<unsigned char> blue(w * h);
//
//        for(int i = 0; i < w * h; ++i) {
//            red[i] = src[i * 3 + 0];
//            green[i] = src[i * 3 + 1];
//            blue[i] = src[i * 3 + 2];
//        }
//
//        auto createTex = [] (GLuint& id, unsigned char* channelData, int w, int h) {
//            glGenTextures(1, &id);
//            glBindTexture(GL_TEXTURE_2D, id);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, channelData);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//            };
//
//        createTex(channelR, red.data(), w, h);
//        createTex(channelG, green.data(), w, h);
//        createTex(channelB, blue.data(), w, h);
//    }
//};
//
//PreviewTexture aoPreview, roughPreview, metallicPreview, ormPreview;
//
//
//bool SaveUnrealAndUnityORM(
//    const std::string& ao, const std::string& rough, const std::string& metal,
//    const std::string& unrealPath, const std::string& unityPath,
//    bool doUnreal, bool doUnity,
//    const std::function<void(float)>& progressCallback = nullptr)
//{
//    int w1, h1, w2, h2, w3, h3;
//    unsigned char* aoData = LoadGrayscale(ao, w1, h1);
//    unsigned char* roughData = LoadGrayscale(rough, w2, h2);
//    unsigned char* metalData = LoadGrayscale(metal, w3, h3);
//
//    if(!aoData || !roughData || !metalData) return false;
//    if(w1 != w2 || w1 != w3 || h1 != h2 || h1 != h3) {
//        std::cerr << "Size mismatch!\n";
//        return false;
//    }
//
//    size_t count = w1 * h1;
//    float totalSteps = 0.0f;
//    if(doUnreal) totalSteps += 2.0f; // loop + write
//    if(doUnity) totalSteps += 2.0f;  // loop + write
//
//    float currentStep = 0.0f;
//
//    if(doUnreal) {
//        std::vector<unsigned char> ormRGB(count * 3);
//        for(size_t i = 0; i < count; ++i) {
//            ormRGB[i * 3 + 0] = aoData[i];
//            ormRGB[i * 3 + 1] = roughData[i];
//            ormRGB[i * 3 + 2] = metalData[i];
//
//            if(progressCallback && (i % (count / 100 + 1) == 0)) {
//                float pixelProgress = static_cast<float>(i) / count;
//                progressCallback((currentStep + pixelProgress) / totalSteps);
//            }
//        }
//        currentStep += 1.0f;
//
//        stbi_write_png(unrealPath.c_str(), w1, h1, 3, ormRGB.data(), w1 * 3);
//        currentStep += 1.0f;
//        if(progressCallback) progressCallback(currentStep / totalSteps);
//
//        ormPreview.Unload();
//        ormPreview.path = unrealPath;
//        ormPreview.width = w1;
//        ormPreview.height = h1;
//        glGenTextures(1, &ormPreview.glId);
//        glBindTexture(GL_TEXTURE_2D, ormPreview.glId);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w1, h1, 0, GL_RGB, GL_UNSIGNED_BYTE, ormRGB.data());
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        ormPreview.GenerateChannelsFromRGB(ormRGB.data(), w1, h1);
//    }
//
//    if(doUnity) {
//        std::vector<unsigned char> ormRGBA(count * 4);
//        for(size_t i = 0; i < count; ++i) {
//            ormRGBA[i * 4 + 0] = metalData[i];
//            ormRGBA[i * 4 + 1] = aoData[i];
//            ormRGBA[i * 4 + 2] = 255;
//            ormRGBA[i * 4 + 3] = 255 - roughData[i];
//
//            if(progressCallback && (i % (count / 100 + 1) == 0)) {
//                float pixelProgress = static_cast<float>(i) / count;
//                progressCallback((currentStep + pixelProgress) / totalSteps);
//            }
//        }
//        currentStep += 1.0f;
//
//        stbi_write_png(unityPath.c_str(), w1, h1, 4, ormRGBA.data(), w1 * 4);
//        currentStep += 1.0f;
//        if(progressCallback) progressCallback(currentStep / totalSteps);
//    }
//
//    stbi_image_free(aoData);
//    stbi_image_free(roughData);
//    stbi_image_free(metalData);
//
//    if(progressCallback) progressCallback(1.0f);
//    return true;
//}
//// === UI ===
//void ShowMainUI()
//{
//    ImGui::SetNextWindowPos(ImVec2(0, 0));
//    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
//    ImGui::Begin("ORMTool", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
//        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
//
//    // --- Header ---
//    ImGui::BeginChild("Header", ImVec2(686, 98), true);
//    ImGui::Text("Action");
//
//    std::string generatedStringButton = "Generated ORM";
//    if (generatingORM)
//    {
//        generatedStringButton = "Canel";
//    }
//
//    if(ImGui::Button(generatedStringButton.c_str(), ImVec2(140, 30)) && !generatingORM) {
//        generatingORM = true;
//        ormProgress = 0.0f;
//
//        std::thread([] {
//            SaveUnrealAndUnityORM(
//                aoPreview.path, roughPreview.path, metallicPreview.path,
//                outputUnreal, outputUnity,
//                generateUnrealORM, generateUnityORM,
//                [] (float progress) {
//                    ormProgress = progress; 
//                }
//            );
//            generatedUnrealPath = outputUnreal;
//            needsPreviewUpdate = true; 
//            generatingORM = false;
//            }).detach();
//
//    }
//
//    if(!generatingORM)
//        ormProgress = 0.0f;
//
//    ImGui::SameLine();
//    ImGui::ProgressBar(ormProgress, ImVec2(522, 30), ormProgress == 0.0f ? "Push Start Generating " : "Generating...");
//
//    if(needsPreviewUpdate) {
//        ormPreview.Unload();
//        ormPreview.path = generatedUnrealPath;
//
//        int w, h, channels;
//        unsigned char* data = stbi_load(generatedUnrealPath.c_str(), &w, &h, &channels, 3);
//        if(data) {
//            ormPreview.width = w;
//            ormPreview.height = h;
//            glGenTextures(1, &ormPreview.glId);
//            glBindTexture(GL_TEXTURE_2D, ormPreview.glId);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//            ormPreview.GenerateChannelsFromRGB(data, w, h);
//            stbi_image_free(data);
//        }
//        needsPreviewUpdate = false;
//    }
//
//    ImGui::Dummy(ImVec2(0, 2));
//
//    ImGui::Checkbox("Generate Unreal ORM (RGB)", &generateUnrealORM);
//    ImGui::SameLine();
//    ImGui::Checkbox("Unity ORM (RGBA)", &generateUnityORM);
//    ImGui::SameLine();
//    ImGui::SetNextItemWidth(150.0f);
//    ImGui::Combo("Channel", (int*)&selectedChannel, "All (RGB)\0AO (R)\0Roughness (G)\0Metallic (B)\0");
//    ImGui::Dummy(ImVec2(0, 2));
//    ImGui::EndChild();
//
//    ImGui::Dummy(ImVec2(0, 2));
//    ImGui::Columns(2, nullptr, false);
//    ImGui::SetColumnWidth(0, 145);
//
//    // === LEFT PANEL ===
//    auto showTextureBlock = [] (const char* label, PreviewTexture& tex, const char* title, int& resolutionIndex, ImVec4 borderColor) 
//        {
//            ImGui::PushID(label);
//            ImGui::TextUnformatted(title);
//            ImGui::SetNextItemWidth(135.0f);
//            ImGui::Combo("##resCombo", &resolutionIndex, resolutionOptions, resolutionCount);
//
//            ImGui::PushStyleColor(ImGuiCol_Button, borderColor);
//            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
//            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
//
//            if(ImGui::ImageButton(label, (ImTextureID)(intptr_t)tex.glId, ImVec2(128, 128))) 
//            {
//                nfdchar_t* outPath = nullptr;
//                if(NFD_OpenDialog("png,jpg", nullptr, &outPath) == NFD_OKAY) 
//                {
//                    tex.Unload();
//                    tex.path = outPath;
//                    tex.Load(outPath);
//                    free(outPath);
//                }
//            }
//
//            ImGui::PopStyleColor(3);
//            ImGui::Dummy(ImVec2(0, 3));
//            ImGui::PopID();
//        };
//
//    showTextureBlock("AO", aoPreview, "AO", aoResolutionIndex, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
//    showTextureBlock("Rough", roughPreview, "Roughness", roughResolutionIndex, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
//    showTextureBlock("Metal", metallicPreview, "Metallic", metalResolutionIndex, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
//
//    ImGui::NextColumn();
//
//    // === RIGHT PANEL ===
//    ImGui::BeginChild("Viewport", ImVec2(0, 544), true);
//    float previewWidth = ImGui::GetContentRegionAvail().x - 1.0f;
//    float aspect = ormPreview.width > 0 ? (float)ormPreview.height / ormPreview.width : 1.0f;
//    float previewHeight = previewWidth * aspect;
//
//    GLuint texId = ormPreview.glId;
//    if(selectedChannel == ORMChannel::AO_R) texId = ormPreview.channelR;
//    else if(selectedChannel == ORMChannel::Roughness_G) texId = ormPreview.channelG;
//    else if(selectedChannel == ORMChannel::Metallic_B) texId = ormPreview.channelB;
//
//    if(texId) {
//        ImGui::Image((ImTextureID)(intptr_t)texId, ImVec2(previewWidth, previewHeight));
//    }
//    else {
//        ImVec2 pos = ImGui::GetCursorScreenPos();
//        ImDrawList* draw_list = ImGui::GetWindowDrawList();
//        draw_list->AddRect(pos, ImVec2(pos.x + previewWidth, pos.y + previewHeight), IM_COL32(255, 255, 255, 255));
//        ImGui::Dummy(ImVec2(previewWidth, previewHeight));
//    }
//
//    ImGui::EndChild();
//    ImGui::Columns(1);
//    ImGui::End();
//}

//// === ENTRY ===
//int main() {
//    stbi_set_flip_vertically_on_load(false);
//    if(!glfwInit()) return -1;
//    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//    GLFWwindow* window = glfwCreateWindow(706, 677, "ORMTool", NULL, NULL);
//    glfwMakeContextCurrent(window);
//    glfwSwapInterval(1);
//
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGui_ImplGlfw_InitForOpenGL(window, true);
//    ImGui_ImplOpenGL3_Init("#version 130");
//    ImGui::StyleColorsDark();
//
//    while(!glfwWindowShouldClose(window)) {
//        glfwPollEvents();
//        ImGui_ImplOpenGL3_NewFrame();
//        ImGui_ImplGlfw_NewFrame();
//        ImGui::NewFrame();
//
//        ShowMainUI();
//
//        ImGui::Render();
//        int display_w, display_h;
//        glfwGetFramebufferSize(window, &display_w, &display_h);
//        glViewport(0, 0, display_w, display_h);
//        glClear(GL_COLOR_BUFFER_BIT);
//        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//        glfwSwapBuffers(window);
//    }
//
//    aoPreview.Unload();
//    roughPreview.Unload();
//    metallicPreview.Unload();
//    ormPreview.Unload();
//
//    ImGui_ImplOpenGL3_Shutdown();
//    ImGui_ImplGlfw_Shutdown();
//    ImGui::DestroyContext();
//    glfwDestroyWindow(window);
//    glfwTerminate();
//    return 0;
//}
#pragma endregion PrototyCode
