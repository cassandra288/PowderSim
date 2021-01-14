#pragma once

#include <string>
#include <vector>


namespace powd::rendering
{
	struct GlVertexAttribute
	{
		unsigned count;
		unsigned type;
		unsigned byteSize;
	};

	class GlShader
	{
	private:
		unsigned program;
		unsigned vShader;
		unsigned fShader;
		unsigned gShader;

		std::string vShaderSource;
		std::string fShaderSource;
		std::string gShaderSource;

		unsigned VAO;
		std::vector<GlVertexAttribute> vertexAttributes;
		unsigned VAOStride;

	public:
		GlShader();
		GlShader(std::string _vShader, std::string _fShader);
		GlShader(std::string _vShader, std::string _fShader, std::string _gShader);
		~GlShader();

		void LoadShader(unsigned _type, std::string _file);
		void LinkShaders();
		void UseProgram();

		void AddAttribute(GlVertexAttribute _attribute);
		void AddAttributes(GlVertexAttribute* _attributes, unsigned _size);
		void BuildVAO();
		unsigned GetVAOStride();
	};
}
