#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

class ShaderProgram
{
public:
	// The program ID
	GLuint shaderProgram;

	// Constructor: read, compile and link shaders to shaderProgram
	ShaderProgram(const GLchar* vertexPath, const GLchar* fragmentPath);
	~ShaderProgram();

	void Use();

private:
	void LoadShader(const GLchar* filename, GLenum type, GLuint &address);
};
