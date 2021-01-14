#include "SandEngine.h"

#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <vector>

#include <CppLog/Logger.h>
#include <GL/glew.h>

#include "src/Core/Exceptions/GenericExceptions.h"
#include "src/Core/Utils/StringUtils.h"

#include "src/Core/Ecs/EntityRegistry.h"
#include "src/Core/Components/CompRenderMaterial.h"
#include "src/Core/Components/CompTransform2D.h"

#include "src/Rendering/Wrappers/GlTexture.h"

USING_LOGGER;


namespace powd::sand
{
	struct PowderTypeData
	{
	public:
		std::string id = "";
		std::string displayName = "";
	};

	namespace
	{
		std::unordered_map<std::string, PowderTypeData> powderTypes;

		entt::entity textureEntity;
		rendering::GlTexture2D* powderTexture;
		rendering::GlMeshID textureMesh;
		rendering::GlShader* textureShader;


#pragma region .ptype parsing
		void ParseLine(std::vector<std::string>& scope, std::string& line, unsigned lineDepth, unsigned lineNum, std::string& file, PowderTypeData& pTypeData)
		{
			std::vector<std::string> symbols;
			std::string symbolBuff;
			bool inQuotes = false;
			bool escaping = false;

			for (unsigned i = 0; i < line.size(); i++)
			{
				bool wasEscaping = escaping;
				switch (line[i])
				{
				case '=':
					if (symbolBuff.size() > 0) symbols.push_back(symbolBuff);
					symbols.push_back("=");
					symbolBuff = "";
					break;

				case '\"':
				case '\'': // quotes
					if (!escaping)
					{
						inQuotes = !inQuotes;
						if (symbolBuff.size() > 0) symbols.push_back(symbolBuff);
					}
					else
						symbolBuff += line[i];
					break;

				case '\\':
					if (escaping)
						symbolBuff += line[i];
					else
						escaping = true;
					break;

				default:
					symbolBuff += line[i];
					break;
				}

				if (wasEscaping)
					escaping = false; // if we started this char escaping, we need to stop escaping afterwards
			}
			if (symbolBuff.size() > 0) symbols.push_back(symbolBuff);

			for (unsigned i = 0; i < symbols.size(); i++)
			{
				if (symbols[i] == "=")
				{
					if (i == 0)
						throw exceptions::GenericException("Missing left hand of = on line [" + std::to_string(lineNum) + "].", __FILE__, __LINE__, "Parse");
					else if (i == symbols.size() - 1)
						throw exceptions::GenericException("Missing right hand of = on line [" + std::to_string(lineNum) + "].", __FILE__, __LINE__, "Parse");

					std::string leftSymbol = symbols[i - 1];
					std::string rightSymbol = symbols[i + 1];

					if (leftSymbol == "ID")
					{
						pTypeData.id = rightSymbol;
					}
					else if (leftSymbol == "Name")
					{
						pTypeData.displayName = rightSymbol;
					}
					else
					{
						throw exceptions::GenericException("Invalid value: \"" + leftSymbol + "\".", __FILE__, __LINE__, "Parse");
					}
				}
			}
		}

		PowderTypeData InterpretPtype(std::string file)
		{
			PowderTypeData pTypeData;

			std::ifstream strm;
			strm.open(file);

			std::vector<std::string> scope;
			std::string currLine;
			bool inQuotes = false;
			bool escaping = false;
			unsigned lineDepth = 0;
			bool lineDepthParsed = false;
			unsigned lineNumber = 0;
			
			try
			{
				auto iterator = std::istreambuf_iterator<char>(strm);
				while (iterator != std::istreambuf_iterator<char>())
				{
					char chr = *iterator;
					iterator++;

					bool wasEscaping = escaping;
					switch (chr)
					{
					case ';': // end statement
						if (inQuotes || escaping)
							currLine += chr;
						else
						{
							ParseLine(scope, currLine, lineDepth, lineNumber++, file, pTypeData);
							currLine = "";
							lineDepth = 0;
							lineDepthParsed = false;
						}
						break;

					case ':': // start a list
						if (inQuotes || escaping)
							currLine += chr;
						else
						{
							scope.push_back(currLine);
							currLine = "";
							lineDepth = 0;
							lineDepthParsed = false;
						}
						break;

					case '-': // if this is the first char in a line, it denotes it is part of the list
						if (currLine.size() != 0 || lineDepth == scope.size())
							currLine += chr;
						else
							lineDepth++;
						break;

					case '\"':
					case '\'': // quotes
						if (!escaping)
							inQuotes = !inQuotes;
						currLine += chr;
						break;

					case '\\': // escape char
						escaping = true;
						currLine += chr;
						break;

					case ' ':
						if (inQuotes)
							currLine += chr;
						break;

					case '\n': // ignore newlines
						break;

					default:
						currLine += chr;
						break;
					}

					if (wasEscaping)
						escaping = false; // if we started this char escaping, we need to stop escaping afterwards

					if (chr != '-' && !lineDepthParsed)
					{
						if (lineDepth < scope.size())
							scope.resize(lineDepth); // TODO: Parse scope correctly
					}
				}

				if (currLine.size() > 0) ParseLine(scope, currLine, lineDepth, lineNumber, file, pTypeData);
			}
			catch (const exceptions::GenericException& e)
			{
				if (e.ExceptionType() != "Parse") throw e;

				std::string msg = utils::string::ReplaceAll(e.Message(), "\n", "\n    ");
				Logger::Lock(Logger::WARNING) << "Invalid ptype file: " << file << ".\nParsing error:\n    " << msg << Logger::endl;
				pTypeData.id = "";
			}

			return pTypeData;
		}
#pragma endregion
	}


	void SandEngineSetup()
	{
#pragma region powder type loading
		{
			namespace fs = std::filesystem;

			for (const auto& file : fs::directory_iterator("TypeData"))
			{
				if (file.is_regular_file() && file.path().extension() == ".ptype")
				{
					PowderTypeData data = InterpretPtype(fs::absolute(file.path()).string());

					if (data.id == "") continue;

					powderTypes[data.id] = data;
				}
			}
		}
#pragma endregion

#pragma region texture creation
		// make a quad
		static const float meshData[24] = {
				-0.5f, -0.5f, 0.f, 0.f,
				0.5f, -0.5f, 1.f, 0.f,
				-0.5f, 0.5f, 0.f, 1.f,
				-0.5f, 0.5f, 0.f, 1.f,
				0.5f, -0.5f, 1.f, 0.f,
				0.5f, 0.5f, 1.f, 1.f
		};
		textureMesh = rendering::GlVertexCache::CreateMesh((void*)meshData, 24 * sizeof(float), {}, 0);

		textureShader = new rendering::GlShader("Shaders/powder_texture.vert", "Shaders/powder_texture.frag");
		textureShader->AddAttribute({ 2, GL_FLOAT, sizeof(float) });
		textureShader->AddAttribute({ 2, GL_FLOAT, sizeof(float) });
		textureShader->BuildVAO();

		powderTexture = new rendering::GlTexture2D(0, rendering::GlTextureFormat::RGB, 128, 72, false);
		powderTexture->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		powderTexture->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		powderTexture->Draw({ 0, 0 }, { 1, 3 }, { 255, 0, 0, 0, 255, 0, 0, 0, 255 });

		textureEntity = ecs::entities.create();
		auto& mat = ecs::entities.emplace<components::CompRenderMaterial>(textureEntity);

		mat.mesh = textureMesh;
		mat.shader = textureShader;
		mat.textures.push_back(powderTexture);

		auto& trans = ecs::entities.emplace<components::CompTransform2D>(textureEntity);

		trans.position = { 0, 0 };
		trans.rotation = 0;
		trans.scale = { 1280, 720 };
#pragma endregion
	}
	void SandEngineShutdown()
	{

	}
}
