#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace powd::rendering
{
	enum class GlTextureFormat
	{
		RGB,
		RGBA
	};

	class GlTexture
	{
	protected:
		unsigned textureId = 0;
		unsigned char textureUnit = 0;
		unsigned  textureType = 0;
		GlTextureFormat textureFormat = GlTextureFormat::RGB;
		unsigned width = 0, height = 0;
		std::string file = "";
		bool mipmap = true;

		unsigned* referenceCounter = nullptr;

		GlTexture(unsigned _textureType, unsigned char _textureUnit, GlTextureFormat _textureFormat, unsigned _width, unsigned _height, bool mipmap);
		GlTexture(GlTexture& _o);
		GlTexture(GlTexture&& _o);
		GlTexture& operator =(GlTexture& _o);
		GlTexture& operator =(GlTexture&& _o);
		~GlTexture();

	public:
		void SetParameter(unsigned _param, int _value);
		void SetTextureUnit(unsigned char _textureUnit);
		void Bind();
		void GenerateMipmaps();

		void LoadFile(std::string _filename);
	};

	class GlTexture2D : public GlTexture
	{
	public:
		GlTexture2D(unsigned char _textureUnit, GlTextureFormat _format, unsigned _width, unsigned _height, bool mipmap = true);
		GlTexture2D(GlTexture2D& _o);
		GlTexture2D(GlTexture2D&& _o);
		GlTexture2D& operator =(GlTexture2D& _o);
		GlTexture2D& operator =(GlTexture2D&& _o);

		void LoadFile(std::string _filename);

		void Draw(glm::ivec2 _off, glm::ivec2 _size, glm::uvec3 _data) { Draw(_off.x, _off.y, _size.x, _size.y, (unsigned char*)glm::value_ptr(_data)); }
		void Draw(glm::ivec2 _off, glm::ivec2 _size, glm::uvec4 _data) { Draw(_off.x, _off.y, _size.x, _size.y, (unsigned char*)glm::value_ptr(_data)); }
		void Draw(int _xOff, int _yOff, int _width, int _height, unsigned char* _data);

		void Read(glm::ivec2 _off, glm::ivec2 _size, void* _dataIn) { Read(_off.x, _off.y, _size.x, _size.y, _dataIn); }
		void Read(int _xOff, int _yOff, int _width, int _height, void* _dataIn);
	};
}
