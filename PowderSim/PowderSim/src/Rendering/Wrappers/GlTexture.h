#pragma once

#include <string>

#include <GL/glew.h>


namespace powd::rendering
{
	class GlTexture
	{
	protected:
		unsigned textureId = 0;
		unsigned char textureUnit = 0;
		unsigned  textureType = 0;
		unsigned width = 0, height = 0;
		std::string file = "";

		unsigned* referenceCounter = nullptr;

		GlTexture(unsigned _textureType, unsigned char _textureUnit, unsigned _width, unsigned _height);
		GlTexture(GlTexture& _o);
		GlTexture(GlTexture&& _o);
		GlTexture& operator =(GlTexture& _o);
		GlTexture& operator =(GlTexture&& _o);
		~GlTexture();

	public:
		void SetParameter(unsigned _param, int _value);
		void SetTextureUnit(unsigned char _textureUnit);
		void Bind();

		void LoadFile(std::string _filename);
	};
}
