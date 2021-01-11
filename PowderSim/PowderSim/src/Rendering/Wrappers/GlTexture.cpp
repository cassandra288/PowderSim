#include "GlTexture.h"

#include <GL/glew.h>


namespace powd::rendering
{
	GlTexture::GlTexture(unsigned _textureType, unsigned char _textureUnit, unsigned _width, unsigned _height)
	{
		glGenTextures(1, &textureId);
		textureUnit = _textureUnit;
		textureType = _textureType;
		width = _width;
		height = _height;
		file = "";

		referenceCounter = new unsigned(1);
	}
	GlTexture::GlTexture(GlTexture& _o)
	{
		textureId = _o.textureId;
		textureUnit = _o.textureUnit;
		textureType = _o.textureType;
		width = _o.width;
		height = _o.height;
		file = std::string(_o.file);

		referenceCounter = _o.referenceCounter;
		*referenceCounter++;
	}
	GlTexture::GlTexture(GlTexture&& _o)
	{
		std::swap(textureId, _o.textureId);
		std::swap(textureUnit, _o.textureUnit);
		std::swap(textureType, _o.textureType);
		std::swap(width, _o.width);
		std::swap(height, _o.height);
		std::swap(file, _o.file);
		std::swap(referenceCounter, _o.referenceCounter);
	}
	GlTexture& GlTexture::operator =(GlTexture& _o)
	{
		textureId = _o.textureId;
		textureUnit = _o.textureUnit;
		textureType = _o.textureType;
		width = _o.width;
		height = _o.height;
		file = std::string(_o.file);

		referenceCounter = _o.referenceCounter;
		*referenceCounter++;
	}
	GlTexture& GlTexture::operator =(GlTexture&& _o)
	{
		std::swap(textureId, _o.textureId);
		std::swap(textureUnit, _o.textureUnit);
		std::swap(textureType, _o.textureType);
		std::swap(width, _o.width);
		std::swap(height, _o.height);
		std::swap(file, _o.file);
		std::swap(referenceCounter, _o.referenceCounter);
	}
	GlTexture::~GlTexture()
	{
		if (referenceCounter != nullptr && *referenceCounter <= 1u)
		{
			delete referenceCounter;
			glDeleteTextures(1, &textureId);
		}
	}


	void GlTexture::SetParameter(unsigned _param, int _value)
	{
		glBindTexture(textureType, textureId);
		glTexParameteri(textureType, _param, _value);
	}
	void GlTexture::SetTextureUnit(unsigned char _textureUnit)
	{
		textureUnit = _textureUnit;
	}
	void GlTexture::Bind()
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(textureType, textureId);
	}
	void GlTexture::LoadFile(std::string _filename)
	{

	}
}
