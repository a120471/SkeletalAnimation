#include "objectShaderProgram.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

ShaderProgram::ShaderProgram(const GLchar* vertexPath, const GLchar* fragmentPath)
{
	// 1. Read and compile shaders
	GLuint vertexShader, fragmentShader;
	this->LoadShader(vertexPath, GL_VERTEX_SHADER, vertexShader);
	this->LoadShader(fragmentPath, GL_FRAGMENT_SHADER, fragmentShader);

	// 2. Link the program
	this->shaderProgram = glCreateProgram();
	glAttachShader(this->shaderProgram, vertexShader);
	glAttachShader(this->shaderProgram, fragmentShader);
	glLinkProgram(this->shaderProgram);
	// Print linking errors if any
	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(this->shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(shaderProgram);
}

void ShaderProgram::LoadShader(const GLchar* filename, GLenum type, GLuint &address)
{
	std::string ShaderString;
	std::ifstream ShaderFile;
	// Ensures ifstream objects can throw exceptions:
	ShaderFile.exceptions(std::ifstream::badbit);
	try
	{
		ShaderFile.open(filename);
		std::stringstream ShaderStream;
		// Read file's buffer contents into streams
		ShaderStream << ShaderFile.rdbuf();
		ShaderFile.close();
		// Convert stream into GLchar array
		ShaderString = ShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const GLchar* ShaderCode = ShaderString.c_str();

	// Compile shaders
	address = glCreateShader(type);
	glShaderSource(address, 1, &ShaderCode, NULL);
	glCompileShader(address);
	// Print compile errors if any
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(address, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(address, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

void ShaderProgram::Use()
{
	glUseProgram(this->shaderProgram);
}