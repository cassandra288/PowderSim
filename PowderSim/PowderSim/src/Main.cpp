#include "Main.h"

#include <thread>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <CppLog/Logger.h>

#include "src/Core/Window/WindowManager.h"
#include "src/Core/Ecs/SystemProto.h"
#include "src/Core/Ecs/SystemProtoFunctions.h"


namespace powd
{
	bool running = true;

	void OnMainClose()
	{
		running = false;
	}

	class TestSystem : ecs::SystemProto
	{
		DEFINE_SYSTEM_PROTO(TestSystem);

	public:
	};
	

	int Main()
	{
		new cpplog::Logger("log.txt", "Main", 4);

		window::StartSDL();
		
		window::mainWindow = window::StartWindow("Powder Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN, OnMainClose);

		while (running)
		{
			cpplog::Logger::Lock() << window::FlushEvents() << cpplog::Logger::endl;

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		return 0;
	}


	IMPLEMENT_SYSTEM_PROTO(TestSystem);
}
