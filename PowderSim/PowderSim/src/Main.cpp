#include "Main.h"

#include <thread>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <CppLog/Logger.h>
#include <SOIL2/SOIL2.h>

#include "src/Core/Window/WindowManager.h"
#include "src/Core/Dispatching/CoreLoops.h"	
#include "src/Rendering/Renderer.h"
#include "src/SandEngine/SandEngine.h"
#include "src/Core/Exceptions/BaseException.h"

#include "src/Core/Utils/StringUtils.h"

#include "src/Core/Ecs/SystemProto.h"
#include "src/Core/Profiling/CPUProfiler.h"

USING_LOGGER


namespace powd
{
	sand::Powder powd;
	void OnStart()
	{
		new cpplog::Logger("log.txt", "Main", 4);

		window::StartSDL();

		window::mainWindow = window::StartWindow("Powder Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN, dispatch::StopCoreLoop);
		
		rendering::StartRenderer();

		sand::SandEngineSetup();

		sand::CreateNewPowder("Test", { 5, 5 }, powd);
	}

	class TestSystem : ecs::SystemProto
	{
		DEFINE_SYSTEM_PROTO(TestSystem);

	private:
		int tmp = 0;

	public:
		System_Tick(dt)
		{
			tmp++;
			tmp = tmp % 30;

			if (tmp == 0)
			{
				sand::TranslatePowderPos({ 1, 0 }, powd);
			}
		}
	};

	void OnStop()
	{
		sand::SandEngineShutdown();

		rendering::StopRenderer();

		window::StopSDL();
	}

	

	int Main()
	{
		try
		{
			OnStart();

			dispatch::CoreLoop();

			OnStop();
		}
		catch (const powd::exceptions::BaseException& e)
		{
			std::string msg = utils::string::ReplaceAll(e.Message(), "\n", "\n    ");
			Logger::Lock(Logger::ERROR) << "Unhandled " << e.ExceptionType() << " Exception at: " << e.File() << "[" << e.Line() << "]\n"
				<< "Message:\n    " << msg << Logger::endl;
		}

		return 0;
	}


	IMPLEMENT_SYSTEM_PROTO(TestSystem);
}
