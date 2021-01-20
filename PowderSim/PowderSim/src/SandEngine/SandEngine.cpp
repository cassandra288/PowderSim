#include "SandEngine.h"

#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>

#include <CppLog/Logger.h>
#include <GL/glew.h>

#include "src/Core/Exceptions/GenericExceptions.h"
#include "src/Core/Utils/StringUtils.h"
#include "src/Core/Utils/DataStructures/DualBlockArray.h"

#include "src/Core/Ecs/SystemProto.h"
#include "src/Core/Ecs/EntityRegistry.h"
#include "src/Core/Components/CompRenderMaterial.h"
#include "src/Core/Components/CompTransform2D.h"

#include "src/Rendering/Wrappers/GlTexture.h"
#include "src/Core/Profiling/CPUProfiler.h"

#include "Behaviours/Behaviours.h"

USING_LOGGER;


namespace powd::sand
{
	struct PowderTypeData
	{
	public:
		std::string id = "";
		std::string displayName = "";
		glm::uvec3 displayColor = { 255, 255, 255 };

		std::vector<std::string> behaviours;
	};

	struct DirtyData
	{
	public:
		glm::uvec2 pos;
		glm::uvec3 color;
	};

	struct PowderData
	{
	public:
		glm::uvec2 position = { 0, 0 };
		std::string typeID = "";

		Powder BlockArrayIndex;
		glm::uvec2 prevPosition = { 0, 0 };

		bool operator ==(const PowderData& _o)
		{ // we dont include any instance-unique data here
			return (
				position == _o.position &&
				typeID == _o.typeID
			);
		}
	};

	namespace
	{
		const unsigned sizeX = 128, sizeY = 72;
		std::unordered_map<std::string, PowderTypeData> powderTypes;

		entt::entity textureEntity;
		rendering::GlTexture2D* powderTexture;
		rendering::GlMeshID textureMesh;
		rendering::GlShader* textureShader;

		std::unordered_map<unsigned long long, Powder> locationMap;
		std::vector<DirtyData> dirtyData;
		std::set<unsigned long long> dirtyPos;

		utils::DualBlockArray<PowderData> powders;
		bool powderChanged = false;


		unsigned long long GetMapLoc(unsigned posX, unsigned posY) { return ((unsigned long long)posX << 32) | posY; }
		unsigned long long GetMapLoc(glm::uvec2 pos) { return ((unsigned long long)pos.x << 32) | pos.y; }

		void CreateDirtyData(Powder powder)
		{
			PowderData& powd = powders[powder];
			dirtyData.push_back({ powd.position, powderTypes[powd.typeID].displayColor });
			dirtyPos.insert(GetMapLoc(powd.position.x, powd.position.y));
			if (powd.prevPosition != powd.position && dirtyPos.find(GetMapLoc(powd.position.x, powd.position.y)) != dirtyPos.end())
				dirtyData.push_back({ powd.prevPosition, glm::uvec3(0, 0, 0) });
		}


#pragma region .ptype parsing
		void ParseLine(std::vector<std::string>& scope, std::string& line, unsigned lineDepth, unsigned lineNum, std::string& file, PowderTypeData& pTypeData)
		{
			std::vector<std::string> symbols;
			std::string symbolBuff;
			bool inQuotes = false;
			bool escaping = false;
			unsigned dashDepth = 0;

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

				case ',':
					if (symbolBuff.size() > 0) symbols.push_back(symbolBuff);
					symbols.push_back(",");
					symbolBuff = "";
					break;

				case '\"':
				case '\'': // quotes
					if (!escaping)
					{
						inQuotes = !inQuotes;
						if (symbolBuff.size() > 0) symbols.push_back(symbolBuff);
						symbolBuff = "";
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

				case '-':
					if (dashDepth < lineDepth)
						dashDepth++;
					else
						symbolBuff += line[i];
					break;

				default:
					symbolBuff += line[i];
					break;
				}

				if (wasEscaping)
					escaping = false; // if we started this char escaping, we need to stop escaping afterwards
			}
			if (symbolBuff.size() > 0) symbols.push_back(symbolBuff);

			if (symbols.size() > 1 && symbols[1] == "=")
			{
				if (symbols.size() == 2)
					throw exceptions::GenericException("Missing right hand of = on line [" + std::to_string(lineNum) + "].", __FILE__, __LINE__, "Parse");

				std::string leftSymbol = symbols[0];
				std::string rightSymbol = symbols[2];

				if (leftSymbol == "ID")
				{
					pTypeData.id = rightSymbol;
				}
				else if (leftSymbol == "Name")
				{
					pTypeData.displayName = rightSymbol;
				}
				else if (leftSymbol == "DisplayColor")
				{
					if (symbols.size() < 7)
						throw exceptions::GenericException("Not enough values for DisplayColor on line [" + std::to_string(lineNum) + "]. 3 Unsigned Integers required.", __FILE__, __LINE__, "Parse");

					unsigned colors[3];
					try
					{
						colors[0] = std::stoul(symbols[2]);
						colors[1] = std::stoul(symbols[4]);
						colors[2] = std::stoul(symbols[6]);
					}
					catch (const std::invalid_argument&) { throw exceptions::GenericException("Invalid value for a color component on line [" + std::to_string(lineNum) + "]. Unsigned Integer required.", __FILE__, __LINE__, "Parse"); }
					catch (const std::out_of_range&) { throw exceptions::GenericException("Invalid value for a color component on line [" + std::to_string(lineNum) + "]. Unsigned Integer required.", __FILE__, __LINE__, "Parse"); }

					pTypeData.displayColor = { colors[0], colors[1], colors[2] };
				}
				else
				{
					throw exceptions::GenericException("Invalid value: \"" + leftSymbol + "\".", __FILE__, __LINE__, "Parse");
				}
			}
			else if (scope.size() > 0 && scope[0] == "Behaviours")
			{
				pTypeData.behaviours.push_back(symbols[0]);
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
							if (!lineDepthParsed)
							{
								if (lineDepth < scope.size())
									scope.resize(lineDepth);
							}

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
							if (!lineDepthParsed)
							{
								if (lineDepth < scope.size())
									scope.resize(lineDepth);
							}

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
				}
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
		LoadBehaviours();

#pragma region powder type loading
		{
			namespace fs = std::filesystem;

			for (const auto& file : fs::recursive_directory_iterator("TypeData"))
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

		powderTexture = new rendering::GlTexture2D(0, rendering::GlTextureFormat::RGB, sizeX, sizeY, false);
		powderTexture->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		powderTexture->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
		ecs::entities.destroy(textureEntity);
		delete textureShader;
		delete powderTexture;
		rendering::GlVertexCache::DeleteMesh(textureMesh);

		UnloadBehaviours();
	}

	
#pragma region Public Powder Modification Functions
	bool CreateNewPowder(std::string id, glm::uvec2 pos, Powder& idOut)
	{
		if (locationMap.find(GetMapLoc(pos.x, pos.y)) != locationMap.end())
		{
			Logger::Lock(Logger::WARNING) << "Attempted to create powder ontop of another powder [Location: (" << pos.x << ", " << pos.y << ") Existing ID: " << locationMap[GetMapLoc(pos.x, pos.y)] << "]" << Logger::endl;
			return false;
		}

		PowderData data;
		data.typeID = id;
		data.position = pos;
		data.prevPosition = pos;
		
		idOut = powders.Insert(data);
		powders[idOut].BlockArrayIndex = idOut;

		CreateDirtyData(idOut);
		locationMap[GetMapLoc(pos)] = idOut;

		Logger::Lock() << "Created powder: " << idOut << Logger::endl;

		return true;
	}
	void RemovePowder(Powder powd)
	{
		Logger::Lock() << "Deleted powder: " << powd << Logger::endl;
		if (!powders.contains(powd))
			return;
		dirtyData.push_back({ powders[powd].position, glm::uvec3(0, 0, 0) });
		locationMap.erase(GetMapLoc(powders[powd].prevPosition));

		powders.RemoveAt(powd);
	}
	void ClearPowders()
	{
		std::vector<Powder> powdersToRemove;
		for (auto pair : locationMap)
			powdersToRemove.push_back(pair.second);

		for (auto powd : powdersToRemove)
			RemovePowder(powd);
	}

	Powder GetPowder(glm::uvec2 pos)
	{
		const auto& findr = locationMap.find(GetMapLoc(pos));
		if (findr == locationMap.end())
			return powders.reservedVal;
		return (*findr).second;
	}
	std::string GetPowderType(Powder powder)
	{
		return powders[powder].typeID;
	}

	glm::uvec2 GetPowderPos(Powder powder)
	{
		return powders[powder].position;
	}
	bool SetPowderPos(glm::uvec2 newPos, Powder powder)
	{
		if (powders[powder].position == newPos)
			return true;

		if (locationMap.find(GetMapLoc(newPos.x, newPos.y)) != locationMap.end())
		{
			Logger::Lock(Logger::WARNING) << "Attempted to move powder ontop of another powder [Location: (" << newPos.x << ", " << newPos.y << ") ID To Modifiy: " << powder << " Existing ID: " << locationMap[GetMapLoc(newPos.x, newPos.y)] << "]" << Logger::endl;
			return false;
		}


		powders[powder].prevPosition = powders[powder].position;
		powders[powder].position = newPos;

		CreateDirtyData(powder);
		locationMap.erase(GetMapLoc(powders[powder].prevPosition));
		locationMap[GetMapLoc(powders[powder].position)] = powders[powder].BlockArrayIndex;

		if (powders.inBlockTwo(powder))
			powders.MoveToBlock1(powder); // wake it up

		return true;
	}
	bool TranslatePowderPos(glm::uvec2 translation, Powder powder)
	{
		return SetPowderPos(GetPowderPos(powder) + translation, powder);
	}
#pragma endregion


	class PowderEngine : ecs::SystemProto
	{
		DEFINE_SYSTEM_PROTO(PowderEngine);

	public:
		System_Tick(dt)
		{
			profiling::StartSectionProfile("Falling Sands");

			/*for (unsigned i = 0; i < powders.blockOneSize(); i++)
			{
				PowderData& powd = powders.data()[i];
				PowderData oldPowd = PowderData(powd);

				PowderTypeData& type = powderTypes[powd.typeID];
				for (std::string behaviour : type.behaviours)
				{
					RunBehaviour(powd.BlockArrayIndex, behaviour);
				}

				if (powd == oldPowd)
					powders.MoveToBlock2(powd.BlockArrayIndex); // this puts it to sleep. Block1 is awake, Block2 is asleep
			}*/ // TODO: Figure out how to not loop chunks and instead just loop existing powders
			RunBuiltinBehaviour("PreTick");
			std::unordered_set<Powder> updatedPowds;
			for (unsigned y = 1; y < sizeY - 1; y++)
			{
				for (unsigned x = 1; x < sizeX - 1; x++)
				{
					unsigned long long mapLoc = GetMapLoc({ x, y });
					
					const auto& mapPos = locationMap.find(mapLoc);
					if (mapPos != locationMap.end() && powders.inBlockOne((*mapPos).second) && updatedPowds.find((*mapPos).second) == updatedPowds.end())
					{
						PowderData& powd = powders[(*mapPos).second];
						PowderData oldPowd = PowderData(powd);

						PowderTypeData& type = powderTypes[powd.typeID];
						bool sleepSkip = false;
						for (std::string behaviour : type.behaviours)
						{
							bool tmp = RunBehaviour(powd.BlockArrayIndex, behaviour);
							if (tmp)
								sleepSkip = true;
						}

						updatedPowds.insert(powd.BlockArrayIndex);
						if (powd == oldPowd && !sleepSkip)
							powders.MoveToBlock2(powd.BlockArrayIndex); // this puts it to sleep. Block1 is awake, Block2 is asleep
					}
				}
			}
			RunBuiltinBehaviour("PostTick");

			for (DirtyData dirtyData : dirtyData)
			{
				powderTexture->Draw(dirtyData.pos, { 1, 1 }, { (unsigned char)dirtyData.color.r, (unsigned char)dirtyData.color.g, (unsigned char)dirtyData.color.b });
			}
			dirtyData.clear();
			dirtyPos.clear();

			profiling::StopSectionProfile();
		}
	};
	IMPLEMENT_SYSTEM_PROTO(PowderEngine)
}
