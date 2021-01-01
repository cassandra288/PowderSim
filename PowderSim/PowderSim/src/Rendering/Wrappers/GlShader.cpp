#include "GlShader.h"

#include <fstream>

#include <GL/glew.h>
#include <CppLog/Logger.h>

#include "src/Core/Exceptions/RenderingExceptions.h"
#include "src/Core/Exceptions/GenericExceptions.h"

USING_LOGGER


namespace powd::rendering
{
	namespace
	{
		std::string LoadShaderSource(std::string _file)
		{
			std::string temp = "";
			std::string src = "";
			std::ifstream file;

			file.open(_file);
			if (file.is_open())
			{
				while (getline(file, temp))
					src += temp + "\n";
			}
			else
				throw exceptions::GenericException("Failed to open shader source file{ " + _file + " }.", __FILE__, __LINE__, "File Access");

			file.close();
			return src;
		}
	}


	GlShader::GlShader()
	{
		program = glCreateProgram();
		vShader = 0;
		fShader = 0;
		gShader = 0;
		vShaderSource = "";
		fShaderSource = "";
		gShaderSource = "";

		glGenBuffers(1, &VAO);
		VAOStride = 0;
	}
	GlShader::GlShader(std::string _vShader, std::string _fShader)
	{
		program = glCreateProgram();
		vShader = 0;
		fShader = 0;
		gShader = 0;
		vShaderSource = "";
		fShaderSource = "";
		gShaderSource = "";

		glGenBuffers(1, &VAO);

		LoadShader(GL_VERTEX_SHADER, _vShader);
		LoadShader(GL_FRAGMENT_SHADER, _fShader);
		LinkShaders();
	}
	GlShader::GlShader(std::string _vShader, std::string _fShader, std::string _gShader)
	{
		program = glCreateProgram();
		vShader = 0;
		fShader = 0;
		gShader = 0;
		vShaderSource = "";
		fShaderSource = "";
		gShaderSource = "";

		glGenBuffers(1, &VAO);

		LoadShader(GL_VERTEX_SHADER, _vShader);
		LoadShader(GL_FRAGMENT_SHADER, _fShader);
		LoadShader(GL_FRAGMENT_SHADER, _gShader);
		LinkShaders();
	}
	GlShader::~GlShader()
	{
		glDeleteProgram(program);
		if (vShader) glDeleteShader(vShader);
		if (fShader) glDeleteShader(fShader);
		if (gShader) glDeleteShader(gShader);

		glDeleteBuffers(1, &VAO);
	}

	void GlShader::LoadShader(unsigned _type, std::string _file)
	{
		char infoLog[512];
		GLint success;

		GLuint shader = glCreateShader(_type);
		std::string src = LoadShaderSource(_file);
		const GLchar* shaderSrc = src.c_str();
		glShaderSource(shader, 1, &shaderSrc, NULL);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			throw exceptions::GenericException("Failed to compile shader {" + _file + "}.\n    Info:\n" + infoLog, __FILE__, __LINE__, "Shader");
		}

		switch (_type)
		{
		case GL_VERTEX_SHADER:
			vShader = shader;
			break;
		case GL_FRAGMENT_SHADER:
			fShader = shader;
			break;
		case GL_GEOMETRY_SHADER:
			gShader = shader;
			break;
		}
	}
	void GlShader::LinkShaders()
	{
		char infoLog[512];
		GLint success;

		glAttachShader(program, vShader);
		glAttachShader(program, fShader);
		if (gShader)
			glAttachShader(program, gShader);

		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			throw exceptions::GenericException("Failed to link shaders.\n    Info:\n" + std::string(infoLog), __FILE__, __LINE__, "Shader");
		}
	}
	void GlShader::UseProgram()
	{
		glUseProgram(program);
		glBindVertexArray(VAO);
	}

	void GlShader::AddAttribute(GlVertexAttribute _attribute)
	{
		vertexAttributes.push_back(_attribute);
	}
	void GlShader::AddAttributes(GlVertexAttribute* _attributes, unsigned _size)
	{
		for (unsigned i = 0; i < _size; i++)
		{
			vertexAttributes.push_back(_attributes[i]);
		}
	}
	void GlShader::BuildVAO()
	{
		if (VAO != 0) glDeleteVertexArrays(1, &VAO);
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		VAOStride = 0;
		for (GlVertexAttribute attrib : vertexAttributes)
		{
			VAOStride += attrib.count * attrib.byteSize;
		}

		unsigned index = 0;
		unsigned offset = 0;
		for (GlVertexAttribute attrib : vertexAttributes)
		{
			glVertexAttribPointer(index, attrib.count, attrib.type, GL_FALSE, VAOStride, (void*)(offset * sizeof(unsigned)));
			glEnableVertexAttribArray(index);
			index++;
			offset += attrib.count;
		}
	}
	unsigned GlShader::GetVAOStride()
	{
		return VAOStride;
	}
}
