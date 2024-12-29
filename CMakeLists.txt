cmake_minimum_required(VERSION 3.15)

project(Blackhole)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 确保引用当前目录下的 conan.cmake
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/conan.cmake")
  message(FATAL_ERROR "conan.cmake not found in the project directory. Please ensure conan.cmake is in the same directory as CMakeLists.txt.")
endif()
include(${CMAKE_SOURCE_DIR}/conan.cmake)

# 配置 Conan，使用新的生成器 CMakeDeps
conan_cmake_configure(REQUIRES glfw/3.3.6 glew/2.2.0 glm/0.9.9.8 stb/cci.20210713
                      GENERATORS CMakeDeps)

# Conan 自动检测配置
conan_cmake_autodetect(settings)

# 替换自动检测到的 Visual Studio 设置为 msvc
list(REMOVE_ITEM settings "compiler=Visual Studio")
list(APPEND settings "compiler=msvc")
list(APPEND settings "compiler.version=193")
list(APPEND settings "compiler.runtime=dynamic")

# 安装 Conan 依赖
conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

# 添加 CMakeDeps 生成的依赖路径
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

# 查找其他依赖库（通过 CMakeDeps 生成）
find_package(glfw3 REQUIRED CONFIG)
find_package(GLEW REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)
find_package(stb REQUIRED CONFIG)
find_package(OpenGL REQUIRED)

# ImGui 源文件和头文件路径
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/src/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

# 收集所有源文件
file(GLOB SRC_FILES
  "${CMAKE_SOURCE_DIR}/src/*.h"
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# 合并 ImGui 源文件和项目源文件
set(ALL_SRC_FILES ${SRC_FILES} ${IMGUI_SOURCES})

# 添加可执行文件
add_executable(${CMAKE_PROJECT_NAME} ${ALL_SRC_FILES})

# 链接依赖库
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE GLEW::GLEW)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE stb::stb)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OpenGL::GL)

# 设置 C++17 标准
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)

# 复制 assets 文件夹到输出目录
add_custom_command(
  TARGET ${CMAKE_PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/assets"
)

# 复制 shader 文件夹到输出目录
add_custom_command(
  TARGET ${CMAKE_PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/shader"
          "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/shader"
)
