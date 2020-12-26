#include "Main.h"

#include <thread>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <CppLog/Logger.h>

#include "WindowManager.h"


namespace powd
{
	bool running = true;

	void OnMainClose()
	{
		running = false;
	}

	int Main()
	{
		new cpplog::Logger("log.txt", "Main", 4);

		window::StartSDL();
		
		window::mainWindow = window::StartWindow("Powder Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN, OnMainClose);

		while (running)
		{
			cpplog::Logger::Lock() << window::FlushEvents() << cpplog::Logger::endl;
		}

		return 0;
	}
}
