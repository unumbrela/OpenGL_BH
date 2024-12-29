#include "shader.h" // 着色器管理头文件

#include <fstream> // 文件输入输出
#include <iostream> // 输入输出流
#include <sstream> // 字符串流
#include <string> // 字符串处理
#include <vector> // 向量容器

#include <GL/glew.h> // GLEW库

// 读取文件内容并返回为字符串
static std::string readFile(const std::string& file) {
    std::string VertexShaderCode;
    std::ifstream ifs(file, std::ios::in); // 打开文件
    if (ifs.is_open()) {
        std::stringstream ss;
        ss << ifs.rdbuf(); // 读取文件内容到字符串流
        return ss.str(); // 返回文件内容字符串
    }
    else {
        throw std::runtime_error("Failed to open file: " + file); // 文件打开失败抛出异常
    }
}

// 编译着色器源代码并返回着色器对象
static GLuint compileShader(const std::string& shaderSource, GLenum shaderType) {
    // 创建着色器对象
    GLuint shader = glCreateShader(shaderType);

    // 设置着色器源代码
    const char* pShaderSource = shaderSource.c_str();
    glShaderSource(shader, 1, &pShaderSource, nullptr);
    glCompileShader(shader); // 编译着色器

    // 检查编译是否成功
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        // 获取错误日志长度
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // 分配存储错误日志的缓冲区
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]); // 获取错误日志
        std::cout << infoLog[0] << std::endl; // 输出错误日志
        glDeleteShader(shader); // 删除失败的着色器对象
        throw std::runtime_error("Failed to compile the shader."); // 抛出编译失败异常
    }

    return shader; // 返回编译成功的着色器对象
}

// 创建着色器程序，链接顶点和片段着色器
GLuint createShaderProgram(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) {
    // 编译顶点着色器
    std::cout << "Compiling vertex shader: " << vertexShaderFile << std::endl;
    GLuint vertexShader = compileShader(readFile(vertexShaderFile), GL_VERTEX_SHADER);

    // 编译片段着色器
    std::cout << "Compiling fragment shader: " << fragmentShaderFile << std::endl;
    GLuint fragmentShader = compileShader(readFile(fragmentShaderFile), GL_FRAGMENT_SHADER);

    // 创建着色器程序对象
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader); // 附加顶点着色器
    glAttachShader(program, fragmentShader); // 附加片段着色器

    // 链接着色器程序
    glLinkProgram(program);
    GLint isLinked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked); // 检查链接状态
    if (isLinked == GL_FALSE) {
        // 获取错误日志长度
        int maxLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        if (maxLength > 0) {
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, NULL, &infoLog[0]); // 获取错误日志
            std::cout << infoLog[0] << std::endl; // 输出错误日志
            throw std::runtime_error("Failed to link the shader."); // 抛出链接失败异常
        }
    }

    // 链接成功后分离和删除着色器对象
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    glDeleteShader(vertexShader); // 删除顶点着色器对象
    glDeleteShader(fragmentShader); // 删除片段着色器对象

    return program; // 返回链接成功的着色器程序对象
}
