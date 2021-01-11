#include "GlTexture.h"

#include <GL/glew.h>
#include <SOIL2/SOIL2.h>

#include "src/Core/Exceptions/GenericExceptions.h"


namespace powd::rendering
{
#pragma region GlTexture
	GlTexture::GlTexture(unsigned _textureType, unsigned char _textureUnit, GlTextureFormat _textureFormat, unsigned _width, unsigned _height, bool _mipmap)
	{
		if (_textureUnit > 15)
			throw exceptions::InvalidArg("Texture Unit can't be above 15.", __FILE__, __LINE__, 1);

		glGenTextures(1, &textureId);
		textureUnit = _textureUnit;
		textureType = _textureType;
		textureFormat = _textureFormat;
		width = _width;
		height = _height;
		file = "";
		mipmap = _mipmap;

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

		return *this;
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

		return *this;
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
		if (_textureUnit > 15)
			throw exceptions::InvalidArg("Texture Unit can't be above 15.", __FILE__, __LINE__, 0);
		textureUnit = _textureUnit;
	}
	void GlTexture::Bind()
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(textureType, textureId);
	}
	void GlTexture::GenerateMipmaps()
	{
		if (mipmap)
			glGenerateMipmap(textureType);
	}

	void GlTexture::LoadFile(std::string _filename)
	{
		file = _filename;
	}
#pragma endregion

#pragma region GlTexture2D
	GlTexture2D::GlTexture2D(unsigned char _textureUnit, GlTextureFormat _format, unsigned _width, unsigned _height, bool mipmap) : GlTexture(GL_TEXTURE_2D, _textureUnit, _format, _width, _height, mipmap)
	{
		glBindTexture(textureType, textureId);
		GLint format;
		unsigned size;
		switch (textureFormat)
		{
		case GlTextureFormat::RGB:
			format = GL_RGB;
			size = 3;
			break;
		case GlTextureFormat::RGBA:
			format = GL_RGBA;
			size = 4;
			break;
		}
		unsigned char* blankPixels = new unsigned char[width * height * size];
		for (unsigned i = 0; i < width * height * size; i++)
			blankPixels[i] = 0;
		glTexImage2D(textureType, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, blankPixels);
		delete[] blankPixels;
	}
	GlTexture2D::GlTexture2D(GlTexture2D& _o) : GlTexture(_o) {}
	GlTexture2D::GlTexture2D(GlTexture2D&& _o) : GlTexture(_o) {}
	GlTexture2D& GlTexture2D::operator =(GlTexture2D& _o) { GlTexture::operator=(_o); return *this; }
	GlTexture2D& GlTexture2D::operator =(GlTexture2D&& _o) { GlTexture::operator=(std::move(_o)); return *this; }

	void GlTexture2D::LoadFile(std::string _filename)
	{
		GlTexture::LoadFile(_filename);

		int newWidth, newHeight, format;
		GLint glFormat;
		switch (textureFormat)
		{
		case GlTextureFormat::RGB:
			format = SOIL_LOAD_RGB;
			glFormat = GL_RGB;
			break;
		case GlTextureFormat::RGBA:
			format = SOIL_LOAD_RGBA;
			glFormat = GL_RGBA;
			break;
		}
		unsigned char* image = SOIL_load_image(file.data(), &newWidth, &newHeight, nullptr, SOIL_LOAD_AUTO);

		if (!image)
			throw exceptions::GenericException("Failed to load image " + file, __FILE__, __LINE__);

		glBindTexture(textureType, textureId);
		glTexImage2D(textureType, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
		if (mipmap)
			glGenerateMipmap(textureType);

		SOIL_free_image_data(image);
	}

	void GlTexture2D::Draw(int _xOff, int _yOff, int _width, int _height, unsigned char* _data)
	{
		GLenum format;
		switch(textureFormat)
		{
		case GlTextureFormat::RGB:
			format = GL_RGB;
			break;
		case GlTextureFormat::RGBA:
			format = GL_RGBA;
			break;
		}

		glBindTexture(textureType, textureId);
		glTexSubImage2D(textureType, 0, _xOff, _yOff, _width, _height, format, GL_UNSIGNED_BYTE, _data);
	}

	void GlTexture2D::Read(int _xOff, int _yOff, int _width, int _height, void* _dataIn)
	{
		GLint format;
		unsigned size;
		switch (textureFormat)
		{
		case GlTextureFormat::RGB:
			format = GL_RGB;
			size = 3;
			break;
		case GlTextureFormat::RGBA:
			format = GL_RGBA;
			size = 4;
			break;
		}

		unsigned dataSize = _width * _height * size;
		dataSize *= sizeof(unsigned char);

		glBindTexture(textureType, textureId);
		glGetTextureSubImage(textureType, 0, _xOff, _yOff, 0, _width, _height, 1, format, GL_UNSIGNED_BYTE, dataSize, _dataIn);
	}
#pragma endregion
}
