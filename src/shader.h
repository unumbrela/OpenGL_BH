#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>

GLuint createShaderProgram(const std::string &vertexShaderFile,
                           const std::string &fragmentShaderFile);

#endif /* SHADER_H */
