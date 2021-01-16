#include "Behaviours.h"

#include <unordered_map>
#include <filesystem>

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include "BehaviourFunctionExports.h"
#include "BehaviourInterface.h"

#include "src/Core/Exceptions/GenericExceptions.h"


namespace powd::sand
{
	namespace
	{
		typedef bool(*ProcType)(unsigned powder);
		typedef void(*BuiltinProcType)();


#ifdef _WINDOWS
		std::unordered_map<std::string, HMODULE> behaviourFiles;
#endif
		std::unordered_map<std::string, ProcType> behaviourFunctions;


		void LoadBehaviourFile(std::string file)
		{
#ifdef _WINDOWS
			HMODULE dllInst = LoadLibraryA(file.data());

			if (dllInst != NULL)
			{
				typedef const char*(*SetupProc_t)(exports_t);

				SetupProc_t setupProc = (SetupProc_t)GetProcAddress(dllInst, "Setup");

				if (setupProc != NULL)
				{
					const char* name = setupProc(exports);

					behaviourFiles[name] = dllInst;
				}
			}
#endif
		}
	}

	void LoadBehaviours()
	{
		Setup();

		namespace fs = std::filesystem;
#ifdef _WINDOWS
		const std::string extensionType = ".dll";
#endif
		for (const auto& file : fs::recursive_directory_iterator("Behaviours"))
		{
			if (file.is_regular_file() && file.path().extension() == extensionType)
			{
				LoadBehaviourFile(fs::absolute(file.path()).string());
			}
		}
	}

	void UnloadBehaviours()
	{
		for (const auto& pair : behaviourFiles)
		{
#ifdef _WINDOWS
			FreeLibrary(pair.second);
#endif
		}
	}


	bool RunBehaviour(unsigned powder, std::string behaviour)
	{
		bool sleepSkip = false;

		if (behaviourFunctions.find(behaviour) != behaviourFunctions.end())
		{
			sleepSkip = behaviourFunctions[behaviour](powder);
		}
		else
		{
			std::string file;
			std::string proc = behaviour;

			{
				size_t delimPos = behaviour.find(':');
				if (delimPos == std::string::npos)
				{
					throw exceptions::GenericException("Attempted to run invalid behaviour \'" + behaviour + "\'.", __FILE__, __LINE__);
				}
				file = behaviour.substr(0, delimPos);
				proc.erase(0, delimPos + 1);
			}

			if (behaviourFiles.find(file) == behaviourFiles.end())
			{
				throw exceptions::GenericException("Attempted to run invalid behaviour \'" + behaviour + "\'. Behaviour file not found \'" + file + "\'", __FILE__, __LINE__);
			}

#ifdef _WINDOWS
			ProcType procAddr = (ProcType)GetProcAddress(behaviourFiles[file], proc.data());

			if (procAddr == NULL)
			{
				throw exceptions::GenericException("Attempted to run invalid behaviour \'" + behaviour + "\'. Behaviour function not found \'" + proc + "\'", __FILE__, __LINE__);
			}

			behaviourFunctions[behaviour] = procAddr;

			sleepSkip = procAddr(powder);
		}
#endif

		return sleepSkip;
	}
	void RunBuiltinBehaviour(std::string behaviour)
	{
		for (auto& pair : behaviourFiles)
		{
			BuiltinProcType procAddr = (BuiltinProcType)GetProcAddress(pair.second, behaviour.data());
			if (procAddr != NULL)
				procAddr();
		}
	}
}
