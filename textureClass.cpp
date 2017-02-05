#include "textureClass.h"

// stb_image, Reference: https://github.com/nothings/stb/blob/master/stb_image.h#L4
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(GLenum textureTarget, const std::string& fileName)
{
	this->textureTarget = textureTarget;
	this->fileName = fileName;
}

Texture::~Texture()
{
	glDeleteTextures(1, &textureObj);
}

bool Texture::Load()
{
	int width, height, comp;
	unsigned char *image = stbi_load(fileName.c_str(), &width, &height, &comp, 0);
	if (image == NULL || (comp != 4 && comp != 3))
		return false;

	// Generate texture ID and load texture data
	glGenTextures(1, &textureObj);

	// Assign texture to ID
	glBindTexture(textureTarget, textureObj);
	if (comp == 4)
		glTexImage2D(textureTarget, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	else if (comp == 3)
		glTexImage2D(textureTarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(textureTarget);

	// Parameters
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(textureTarget, 0);
	stbi_image_free(image);
		
	return true;
}

void Texture::Bind(GLenum textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(textureTarget, textureObj);
}
