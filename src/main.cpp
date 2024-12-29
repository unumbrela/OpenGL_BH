#include <cuda_runtime.h> // CUDA运行时
#include <assert.h>
#include <map>
#include <stdio.h>
#include <vector>

#include <GL/glew.h> // GLEW库
#include <GLFW/glfw3.h> // GLFW库
#include <glm/glm.hpp> // GLM数学库
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h> // ImGui库

#include "GLDebugMessageCallback.h" // OpenGL调试回调
#include "imgui_impl_glfw.h" // ImGui GLFW绑定
#include "imgui_impl_opengl3.h" // ImGui OpenGL绑定
#include "render.h" // 渲染相关
#include "shader.h" // 着色器管理
#include "texture.h" // 纹理管理

// 包含irrKlang头文件用于音频
#include <irrKlang.h>

static const int SCR_WIDTH = 1920; // 屏幕宽度
static const int SCR_HEIGHT = 1080; // 屏幕高度

static float mouseX, mouseY; // 鼠标位置

// 定义ImGui复选框宏
#define IMGUI_TOGGLE(NAME, DEFAULT)                                            \
  static bool NAME = DEFAULT;                                                  \
  ImGui::Checkbox(#NAME, &NAME);                                               \
  rtti.floatUniforms[#NAME] = NAME ? 1.0f : 0.0f;

// 定义ImGui滑动条宏
#define IMGUI_SLIDER(NAME, DEFAULT, MIN, MAX)                                  \
  static float NAME = DEFAULT;                                                 \
  ImGui::SliderFloat(#NAME, &NAME, MIN, MAX);                                  \
  rtti.floatUniforms[#NAME] = NAME;

static void glfwErrorCallback(int error, const char* description) {
    // GLFW错误回调函数，输出错误信息
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void mouseCallback(GLFWwindow* window, double x, double y) {
    // 鼠标移动回调函数，更新鼠标位置
    static float lastX = 400.0f;
    static float lastY = 300.0f;
    static float yaw = 0.0f;
    static float pitch = 0.0f;
    static float firstMouse = true;

    mouseX = (float)x;
    mouseY = (float)y;
}

class PostProcessPass {
private:
    GLuint program; // 着色器程序

public:
    // 构造函数，创建着色器程序
    PostProcessPass(const std::string& fragShader) {
        this->program = createShaderProgram("shader/simple.vert", fragShader);

        glUseProgram(this->program);
        glUniform1i(glGetUniformLocation(program, "texture0"), 0); // 设置纹理单元
        glUseProgram(0);
    }

    // 渲染后处理
    void render(GLuint inputColorTexture, GLuint destFramebuffer = 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, destFramebuffer); // 绑定帧缓冲

        glDisable(GL_DEPTH_TEST); // 禁用深度测试

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // 清除颜色为红色
        glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲

        glUseProgram(this->program); // 使用着色器程序

        // 设置分辨率Uniform
        glUniform2f(glGetUniformLocation(this->program, "resolution"),
            (float)SCR_WIDTH, (float)SCR_HEIGHT);

        // 设置时间Uniform
        glUniform1f(glGetUniformLocation(this->program, "time"),
            (float)glfwGetTime());

        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, inputColorTexture); // 绑定输入颜色纹理

        glDrawArrays(GL_TRIANGLES, 0, 6); // 绘制两组三角形

        glUseProgram(0); // 解绑着色器程序
    }
};

int main(int, char**) {
    // 设置CUDA设备
    cudaSetDevice(1);
    // 设置GLFW错误回调
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
        return 1;

    // 创建窗口并设置图形上下文
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // 无边框窗口
    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Wormhole", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window); // 设置当前上下文
    glfwSwapInterval(1); // 启用垂直同步
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback); // 设置鼠标回调
    glfwSetWindowPos(window, 0, 0); // 设置窗口位置

    // 初始化GLEW
    bool err = glewInit() != GLEW_OK;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    if (0)
    {
        // 启用OpenGL调试层
        //
        // GL_DEBUG_OUTPUT - 更快的版本，但不适用于断点
        // GL_DEBUG_OUTPUT_SYNCHRONOUS - 回调与错误同步，可以在回调中设置断点获取堆栈跟踪

        glEnable(GL_DEBUG_OUTPUT);
        // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // 设置调试消息回调函数
        glDebugMessageCallback(GLDebugMessageCallback, nullptr);
    }

    {
        // 选择GL和GLSL版本
#if __APPLE__
    // 针对苹果的设置：GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 仅限3.2+
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 苹果要求
#else
    // 其他平台设置：GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 仅限3.2+
        // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 仅限3.0+
#endif

        // 初始化ImGui上下文
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        // 设置ImGui样式
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // 初始化ImGui平台和渲染器绑定
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // ImGui窗口状态
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    }

    // 创建黑洞帧缓冲和纹理
    GLuint fboBlackhole, texBlackhole;
    texBlackhole = createColorTexture(SCR_WIDTH, SCR_HEIGHT);

    FramebufferCreateInfo info = {};
    info.colorTexture = texBlackhole;
    if (!(fboBlackhole = createFramebuffer(info))) {
        assert(false); // 确认帧缓冲创建成功
    }

    // 创建全屏四边形VAO
    GLuint quadVAO = createQuadVAO();
    glBindVertexArray(quadVAO);

    // 初始化irrKlang声音引擎
    irrklang::ISoundEngine* soundEngine = irrklang::createIrrKlangDevice();
    if (!soundEngine) {
        fprintf(stderr, "Failed to initialize sound engine!\n");
        return 1;
    }

    // 播放背景音乐，循环播放
    irrklang::ISound* music = soundEngine->play2D("assets/bgm.wav", true, false, true);
    if (!music) {
        fprintf(stderr, "Failed to load background music!\n");
        soundEngine->drop(); // 释放声音引擎
        return 1;
    }

    // 主循环前的初始化
    PostProcessPass passthrough("shader/passthrough.frag"); // 后处理通过

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // 处理事件

        // 开始新的ImGui帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow(); // 显示ImGui示例窗口

        int width, height;
        glfwGetFramebufferSize(window, &width, &height); // 获取窗口大小
        glViewport(0, 0, width, height); // 设置视口

        // renderScene(fboBlackhole); // 渲染场景到帧缓冲（注释掉）

        // 加载纹理资源
        static GLuint galaxy = loadCubemap("assets/skybox_nebula_dark");
        static GLuint colorMap = loadTexture2D("assets/color_map.png");
        static GLuint uvChecker = loadTexture2D("assets/uv_checker.png");

        // 确保texBlackhole不被重新定义
        // 移除以下行以防止重新定义
        // static GLuint texBlackhole = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            // 设置渲染到纹理的信息
            RenderToTextureInfo rtti;
            rtti.fragShader = "shader/blackhole_main.frag"; // 使用黑洞主片段着色器
            rtti.cubemapUniforms["galaxy"] = galaxy; // 设置立方体贴图
            rtti.textureUniforms["colorMap"] = colorMap; // 设置颜色贴图
            rtti.floatUniforms["mouseX"] = mouseX; // 设置鼠标X位置
            rtti.floatUniforms["mouseY"] = mouseY; // 设置鼠标Y位置
            rtti.targetTexture = texBlackhole; // 目标纹理
            rtti.width = SCR_WIDTH; // 纹理宽度
            rtti.height = SCR_HEIGHT; // 纹理高度

            // 使用ImGui宏添加界面控件
            // IMGUI_TOGGLE(gravitationalLensing, true);
            IMGUI_TOGGLE(renderBlackHole, true);
            IMGUI_TOGGLE(mouseControl, true);
            IMGUI_SLIDER(cameraRoll, 0.0f, -180.0f, 180.0f);
            IMGUI_TOGGLE(frontView, false);
            IMGUI_TOGGLE(topView, false);
            IMGUI_TOGGLE(adiskEnabled, true);
            IMGUI_TOGGLE(adiskParticle, true);
            IMGUI_SLIDER(adiskDensityV, 2.0f, 0.0f, 10.0f);
            IMGUI_SLIDER(adiskDensityH, 4.0f, 0.0f, 10.0f);
            IMGUI_SLIDER(adiskHeight, 0.55f, 0.0f, 1.0f);
            IMGUI_SLIDER(adiskLit, 0.25f, 0.0f, 4.0f);
            IMGUI_SLIDER(adiskNoiseLOD, 5.0f, 1.0f, 12.0f);
            IMGUI_SLIDER(adiskNoiseScale, 0.8f, 0.0f, 10.0f);
            IMGUI_SLIDER(adiskSpeed, 0.5f, 0.0f, 1.0f);

            renderToTexture(rtti); // 渲染到纹理
        }

        // 创建亮度纹理
        static GLuint texBrightness = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti;
            rtti.fragShader = "shader/bloom_brightness_pass.frag"; // 亮度提取着色器
            rtti.textureUniforms["texture0"] = texBlackhole; // 输入纹理
            rtti.targetTexture = texBrightness; // 目标纹理
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;
            renderToTexture(rtti); // 渲染到纹理
        }

        const int MAX_BLOOM_ITER = 8; // 最大Bloom迭代次数
        static GLuint texDownsampled[MAX_BLOOM_ITER] = { 0 }; // 下采样纹理数组
        static GLuint texUpsampled[MAX_BLOOM_ITER] = { 0 }; // 上采样纹理数组
        if (texDownsampled[0] == 0) {
            // 初始化下采样和上采样纹理
            for (int i = 0; i < MAX_BLOOM_ITER; i++) {
                texDownsampled[i] =
                    createColorTexture(SCR_WIDTH >> (i + 1), SCR_HEIGHT >> (i + 1));
                texUpsampled[i] = createColorTexture(SCR_WIDTH >> i, SCR_HEIGHT >> i);
            }
        }

        static int bloomIterations = MAX_BLOOM_ITER; // 当前Bloom迭代次数
        ImGui::SliderInt("bloomIterations", &bloomIterations, 1, 8); // ImGui滑动条调整迭代次数
        for (int level = 0; level < bloomIterations; level++) {
            // 下采样过程
            RenderToTextureInfo rtti;
            rtti.fragShader = "shader/bloom_downsample.frag"; // 下采样着色器
            rtti.textureUniforms["texture0"] =
                level == 0 ? texBrightness : texDownsampled[level - 1]; // 输入纹理
            rtti.targetTexture = texDownsampled[level]; // 目标下采样纹理
            rtti.width = SCR_WIDTH >> (level + 1); // 缩小宽度
            rtti.height = SCR_HEIGHT >> (level + 1); // 缩小高度
            renderToTexture(rtti); // 渲染到纹理
        }

        for (int level = bloomIterations - 1; level >= 0; level--) {
            // 上采样过程
            RenderToTextureInfo rtti;
            rtti.fragShader = "shader/bloom_upsample.frag"; // 上采样着色器
            rtti.textureUniforms["texture0"] = level == bloomIterations - 1
                ? texDownsampled[level]
                : texUpsampled[level + 1]; // 输入纹理
            rtti.textureUniforms["texture1"] =
                level == 0 ? texBrightness : texDownsampled[level - 1]; // 另一个输入纹理
            rtti.targetTexture = texUpsampled[level]; // 目标上采样纹理
            rtti.width = SCR_WIDTH >> level; // 缩放宽度
            rtti.height = SCR_HEIGHT >> level; // 缩放高度
            renderToTexture(rtti); // 渲染到纹理
        }

        // 创建最终Bloom合成纹理
        static GLuint texBloomFinal = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti;
            rtti.fragShader = "shader/bloom_composite.frag"; // Bloom合成着色器
            rtti.textureUniforms["texture0"] = texBlackhole; // 原始纹理
            rtti.textureUniforms["texture1"] = texUpsampled[0]; // Bloom纹理
            rtti.targetTexture = texBloomFinal; // 目标合成纹理
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;

            IMGUI_SLIDER(bloomStrength, 0.1f, 0.0f, 1.0f); // 调整Bloom强度

            renderToTexture(rtti); // 渲染到纹理
        }

        // 创建色调映射纹理
        static GLuint texTonemapped = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RenderToTextureInfo rtti;
            rtti.fragShader = "shader/tonemapping.frag"; // 色调映射着色器
            rtti.textureUniforms["texture0"] = texBloomFinal; // 输入纹理
            rtti.targetTexture = texTonemapped; // 目标纹理
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;

            IMGUI_TOGGLE(tonemappingEnabled, true); // 启用/禁用色调映射
            IMGUI_SLIDER(gamma, 2.5f, 1.0f, 4.0f); // 调整Gamma值

            renderToTexture(rtti); // 渲染到纹理
        }

        passthrough.render(texTonemapped); // 后处理渲染

        ImGui::Render(); // 渲染ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // 绘制ImGui数据

        glfwSwapBuffers(window); // 交换缓冲区
    }

    // 清理irrKlang声音引擎
    if (soundEngine) {
        soundEngine->drop(); // 释放声音引擎
    }

    // 清理ImGui资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window); // 销毁窗口
    glfwTerminate(); // 终止GLFW

    return 0;
}
