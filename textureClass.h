#pragma once

#include <string>

#define GLEW_STATIC
#include <GL/glew.h>

class Texture
{
public:
	Texture(GLenum textureTarget, const std::string& fileName);
	~Texture();

	bool Load();

	void Bind(GLenum textureUnit);

private:
	std::string fileName;
	GLenum textureTarget;
	GLuint textureObj;
};