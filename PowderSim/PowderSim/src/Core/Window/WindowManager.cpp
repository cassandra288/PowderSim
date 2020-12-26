#include "WindowManager.h"

#include <unordered_map>
#include <CppLog/Logger.h>
USING_LOGGER


namespace powd::window
{
	uint32_t mainWindow;

	namespace
	{
		std::unordered_map<uint32_t, Window*> instanceMap;

		bool sdlStarted = false;
	}

	Window* GetInstance(uint32_t _id)
	{
		if (instanceMap.find(_id) == instanceMap.end()) /// check to see if the ID is in the map
			return nullptr;
		return instanceMap[_id];
	}

	uint32_t StartWindow(
		std::string _windowTitle,
		uint32_t _windowXPos,
		uint32_t _windowYPos,
		uint32_t _windowWidth,
		uint32_t _windowHeight,
		uint32_t _windowFlags,
		void(*_windowClosedCallback)()
	)
	{
		if (!sdlStarted)
		{
			Logger::Log("Creating window without starting SDL.", Logger::WARNING);
			StartSDL();
		}

		Window* window = new Window(_windowTitle, _windowXPos, _windowYPos, _windowWidth, _windowHeight, _windowFlags, _windowClosedCallback);
		instanceMap[window->GetID()] = window;
		return window->GetID();
	}

	void StopWindow(uint32_t _id)
	{
		if (instanceMap.find(_id) == instanceMap.end())
			return Logger::Log("Attempted to stop an invalid window.", Logger::WARNING);
		delete instanceMap[_id];
		instanceMap.erase(_id);
	}

	void StartSDL(uint32_t _flags)
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | _flags) != 0)
			Logger::Log("SDL Error: " + std::string(SDL_GetError()), Logger::EXCEPTION);

		sdlStarted = true;
	}

	void StopSDL()
	{
		SDL_Quit();

		sdlStarted = false;
	}

	
	int FlushEvents()
	{
		SDL_PumpEvents();

		SDL_Event e;
		int currentEvents = 0;
		const int maxEvents = 50;
		while (SDL_PollEvent(&e) && currentEvents < maxEvents) // poll events
		{
			currentEvents++;

			if (e.type == SDL_WINDOWEVENT)
			{
				if (instanceMap.find(e.window.windowID) == instanceMap.end())
					continue;
				Window* relevantWindow = GetInstance(e.window.windowID);
				switch (e.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					relevantWindow->width = e.window.data1;
					relevantWindow->height = e.window.data2;
					break;

				case SDL_WINDOWEVENT_MINIMIZED:
					relevantWindow->minimized = true;
					break;

				case SDL_WINDOWEVENT_RESTORED:
					relevantWindow->minimized = false;
					break;

				case SDL_WINDOWEVENT_CLOSE:
					relevantWindow->closedCallback();
					StopWindow(e.window.windowID);
					break;
				}
			}
		}

		return SDL_PeepEvents(nullptr, UINT32_MAX, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
	}
}
