#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <string>

GLuint loadTexture2D(const std::string &file, bool repeat = true);

GLuint loadCubemap(const std::string &cubemapDir);

#endif /* TEXTURE_H */
