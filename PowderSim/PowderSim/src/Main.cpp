#include "Main.h"

#include <thread>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <CppLog/Logger.h>

#include "src/Core/Window/WindowManager.h"
#include "src/Core/Ecs/SystemProto.h"
#include "src/Core/Dispatching/CoreLoops.h"

USING_LOGGER


namespace powd
{
	class TestSystem : ecs::SystemProto
	{
		DEFINE_SYSTEM_PROTO(TestSystem);

	public:
		System_Tick(dt)
		{
			Logger::Lock() << "Tick: " << dt << Logger::endl;
		}

		System_Render(dt)
		{
			Logger::Lock() << "Render: " << dt << Logger::endl;
		}
	};
	

	int Main()
	{
		new cpplog::Logger("log.txt", "Main", 4);

		window::StartSDL();
		
		window::mainWindow = window::StartWindow("Powder Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN, dispatch::StopCoreLoop);

		dispatch::CoreLoop();

		return 0;
	}


	IMPLEMENT_SYSTEM_PROTO(TestSystem);
}
