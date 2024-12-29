#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <GL/glew.h>

static std::string readFile(const std::string& file) {
    std::string VertexShaderCode;
    std::ifstream ifs(file, std::ios::in);
    if (ifs.is_open()) {
        std::stringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }
    else {
        throw std::runtime_error("Failed to open file: " + file);
    }
}

static GLuint compileShader(const std::string& shaderSource,
    GLenum shaderType) {
    // 创建着色器
    GLuint shader = glCreateShader(shaderType);

    // 编译着色器
    const char* pShaderSource = shaderSource.c_str();
    glShaderSource(shader, 1, &pShaderSource, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {

        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // maxLength 包括 NULL 字符
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
        std::cout << infoLog[0] << std::endl;
        glDeleteShader(shader);
        throw std::runtime_error("Failed to compile the shader.");
    }

    return shader;
}

GLuint createShaderProgram(const std::string& vertexShaderFile,
    const std::string& fragmentShaderFile) {

    // 编译顶点和片段着色器
    std::cout << "Compiling vertex shader: " << vertexShaderFile << std::endl;
    GLuint vertexShader =
        compileShader(readFile(vertexShaderFile), GL_VERTEX_SHADER);

    std::cout << "Compiling fragment shader: " << fragmentShaderFile << std::endl;
    GLuint fragmentShader =
        compileShader(readFile(fragmentShaderFile), GL_FRAGMENT_SHADER);

    // 创建着色器程序
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // 链接程序
    glLinkProgram(program);
    GLint isLinked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        int maxLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        if (maxLength > 0) {
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, NULL, &infoLog[0]);
            std::cout << infoLog[0] << std::endl;
            throw std::runtime_error("Failed to link the shader.");
        }
    }

    // 成功链接后分离着色器
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
