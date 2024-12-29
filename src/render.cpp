#include "render.h" // 渲染相关头文件
#include "shader.h" // 着色器管理头文件

#include <iostream> // 输入输出流

#include <GLFW/glfw3.h> // GLFW库
#include <glm/glm.hpp> // GLM数学库

// 创建颜色纹理
GLuint createColorTexture(int width, int height, bool hdr) {
    GLuint colorTexture;
    glGenTextures(1, &colorTexture); // 生成纹理对象

    glBindTexture(GL_TEXTURE_2D, colorTexture); // 绑定纹理
    glTexImage2D(GL_TEXTURE_2D, 0, hdr ? GL_RGB16F : GL_RGB, width, height, 0,
        GL_RGB, hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, NULL); // 定义纹理图像
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // 设置缩小过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // 设置放大过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // 重复设置缩小过滤（可能为冗余）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // 重复设置放大过滤（可能为冗余）

    return colorTexture; // 返回生成的纹理ID
}

// 创建帧缓冲对象
GLuint createFramebuffer(const FramebufferCreateInfo& info) {
    GLuint framebuffer;

    // 生成并绑定新的帧缓冲对象
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // 绑定颜色附件
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        info.colorTexture, 0);

    if (info.createDepthBuffer) {
        // 创建用于深度和模板的渲染缓冲对象
        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, info.width,
            info.height); // 定义渲染缓冲存储
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, rbo); // 绑定渲染缓冲到帧缓冲
    }

    // 检查帧缓冲是否完整
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 解绑帧缓冲
        return 0; // 返回0表示创建失败
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 解绑帧缓冲

    return framebuffer; // 返回生成的帧缓冲ID
}

// 创建全屏四边形的VAO
GLuint createQuadVAO() {
    std::vector<glm::vec3> vertices;

    // 定义两个三角形组成的四边形顶点
    vertices.push_back(glm::vec3(-1, -1, 0));
    vertices.push_back(glm::vec3(-1, 1, 0));
    vertices.push_back(glm::vec3(1, 1, 0));

    vertices.push_back(glm::vec3(1, 1, 0));
    vertices.push_back(glm::vec3(1, -1, 0));
    vertices.push_back(glm::vec3(-1, -1, 0));

    // 生成并绑定顶点数组对象
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 生成并绑定顶点缓冲对象
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
        &vertices[0], GL_STATIC_DRAW); // 上传顶点数据

    // 设置顶点属性指针
    glEnableVertexAttribArray(0); // 启用位置属性
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0,        // 属性位置
        3,        // 每个顶点属性的大小
        GL_FLOAT, // 数据类型
        GL_FALSE, // 是否标准化
        0,        // 步长
        (void*)0 // 偏移量
    );

    glBindVertexArray(0); // 解绑VAO

    return vao; // 返回生成的VAO ID
}

// 绑定纹理到指定的纹理单元
static bool bindToTextureUnit(GLuint program, const std::string& name,
    GLenum textureType, GLuint texture,
    int textureUnitIndex) {
    GLint loc = glGetUniformLocation(program, name.c_str()); // 获取Uniform位置
    if (loc != -1) {
        glUniform1i(loc, textureUnitIndex); // 设置Uniform为纹理单元索引

        // 激活并绑定纹理
        glActiveTexture(GL_TEXTURE0 + textureUnitIndex);
        glBindTexture(textureType, texture);
        return true; // 绑定成功
    }
    else {
        std::cout << "WARNING: uniform " << name << " is not found in shader"
            << std::endl;
        return false; // 绑定失败
    }
}

// 渲染到纹理
void renderToTexture(const RenderToTextureInfo& rtti) {
    // 延迟创建帧缓冲并将纹理附加为颜色附件
    static std::map<GLuint, GLuint> textureFramebufferMap;
    GLuint targetFramebuffer;
    if (!textureFramebufferMap.count(rtti.targetTexture)) {
        FramebufferCreateInfo createInfo;
        createInfo.colorTexture = rtti.targetTexture;
        targetFramebuffer = createFramebuffer(createInfo); // 创建帧缓冲
        textureFramebufferMap[rtti.targetTexture] = targetFramebuffer; // 存储映射
    }
    else {
        targetFramebuffer = textureFramebufferMap[rtti.targetTexture]; // 获取已有帧缓冲
    }

    // 延迟加载着色器程序
    static std::map<std::string, GLuint> shaderProgramMap;
    GLuint program;
    if (!shaderProgramMap.count(rtti.fragShader)) {
        program = createShaderProgram(rtti.vertexShader, rtti.fragShader); // 创建着色器程序
        shaderProgramMap[rtti.fragShader] = program; // 存储映射
    }
    else {
        program = shaderProgramMap[rtti.fragShader]; // 获取已有程序
    }

    // 渲染全屏四边形
    {
        glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer); // 绑定目标帧缓冲

        glViewport(0, 0, rtti.width, rtti.height); // 设置视口大小

        glDisable(GL_DEPTH_TEST); // 禁用深度测试

        glClearColor(0.0f, 1.0f, 1.0f, 1.0f); // 设置清除颜色为青色
        glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲

        glUseProgram(program); // 使用着色器程序

        // 设置Uniform变量
        {
            glUniform2f(glGetUniformLocation(program, "resolution"),
                (float)rtti.width, (float)rtti.height); // 设置分辨率

            glUniform1f(glGetUniformLocation(program, "time"), (float)glfwGetTime()); // 设置时间

            // 更新浮点型Uniform变量
            for (auto const& [name, val] : rtti.floatUniforms) {
                GLint loc = glGetUniformLocation(program, name.c_str());
                if (loc != -1) {
                    glUniform1f(loc, val); // 设置Uniform值
                }
                else {
                    std::cout << "WARNING: uniform " << name << " is not found"
                        << std::endl;
                }
            }

            // 更新纹理Uniform变量
            int textureUnit = 0;
            for (auto const& [name, texture] : rtti.textureUniforms) {
                bindToTextureUnit(program, name, GL_TEXTURE_2D, texture, textureUnit++); // 绑定2D纹理
            }
            for (auto const& [name, texture] : rtti.cubemapUniforms) {
                bindToTextureUnit(program, name, GL_TEXTURE_CUBE_MAP, texture,
                    textureUnit++); // 绑定立方体贴图
            }
        }

        glDrawArrays(GL_TRIANGLES, 0, 6); // 绘制两组三角形

        glUseProgram(0); // 解绑着色器程序
    }
}
