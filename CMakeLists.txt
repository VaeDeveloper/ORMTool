cmake_minimum_required(VERSION 3.16)
project(ORMTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)

# -----------------------
# Output folders
# -----------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

message(STATUS "📦 Starting to fetch dependencies...")

# GLFW
message(STATUS "⬇️  Fetching GLFW...")
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        latest
)
FetchContent_MakeAvailable(glfw)
message(STATUS "✅ GLFW is ready!")

# Dear ImGui
message(STATUS "⬇️  Fetching Dear ImGui...")
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

message(STATUS "✅ Dear ImGui is ready!")

# NativeFileDialog
message(STATUS "⬇️  Fetching Native File Dialog...")

FetchContent_Declare(
  nativefiledialog
  GIT_REPOSITORY https://github.com/mlabbe/nativefiledialog.git
  GIT_TAG master
)
if(NOT nativefiledialog_POPULATED)
  message(STATUS "🔽 Downloading NativeFileDialog from GitHub...")
endif()
FetchContent_MakeAvailable(nativefiledialog)

if(WIN32)
  set(NFD_SRC
    ${nativefiledialog_SOURCE_DIR}/src/nfd_common.c
    ${nativefiledialog_SOURCE_DIR}/src/nfd_win.cpp
  )
elseif(APPLE)
  set(NFD_SRC
    ${nativefiledialog_SOURCE_DIR}/src/nfd_common.c
    ${nativefiledialog_SOURCE_DIR}/src/nfd_cocoa.m
  )
elseif(UNIX)
  set(NFD_SRC
    ${nativefiledialog_SOURCE_DIR}/src/nfd_common.c
    ${nativefiledialog_SOURCE_DIR}/src/nfd_gtk.c
  )
endif()

add_library(nfd STATIC ${NFD_SRC})
target_include_directories(nfd PUBLIC ${nativefiledialog_SOURCE_DIR}/src/include)
target_compile_definitions(nfd PRIVATE _CRT_SECURE_NO_WARNINGS)
message(STATUS "✅ Native File Dialog is ready!")

# stb_image implementation flags
add_compile_definitions(
    STB_IMAGE_IMPLEMENTATION
    STB_IMAGE_WRITE_IMPLEMENTATION
)

# ImGui backends
set(IMGUI_BACKENDS
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Executable
add_executable(ORMTool
    src/main.cpp

    src/App/App.h
    src/App/App.cpp

    src/MVC/IView.h
    src/MVC/IController.h
    src/MVC/IModel.h
    src/MVC/BoilerplateMacro.h

    src/UI/UIManager.cpp
    src/UI/UIManager.h

    src/IO/IOService.cpp
    src/IO/IOService.h


)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source" FILES
    src/main.cpp

    src/App/App.h
    src/App/App.cpp

    src/MVC/IView.h
    src/MVC/IController.h
    src/MVC/IModel.h
    src/MVC/BoilerplateMacro.h

    src/UI/UIManager.cpp
    src/UI/UIManager.h
    
    src/IO/IOService.cpp
    src/IO/IOService.h
)


# Include directories
target_include_directories(ORMTool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/stb
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_BINARY_DIR}/_deps/nativefiledialog-src/src/include
)

# Link libraries
find_package(OpenGL REQUIRED)

target_link_libraries(ORMTool PRIVATE
    imgui
    glfw
    OpenGL::GL
    nfd
)

# Set default startup project in Visual Studio
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ORMTool)
endif()


message(STATUS "🚀 Configuring ORMTool project")
message(STATUS "🧠 Using C++ standard: ${CMAKE_CXX_STANDARD}")
if(WIN32)
  message(STATUS "🖥  Platform: Windows")
elseif(UNIX AND NOT APPLE)
  message(STATUS "🐧 Platform: Linux")
elseif(APPLE)
  message(STATUS "🍎 Platform: macOS")
endif()
message(STATUS "🎉 ✅ All dependencies are ready. ORMTool is ready to build!")