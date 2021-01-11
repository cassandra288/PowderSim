#include "Window.h"


namespace powd::window
{
	Window::Window(
		std::string _windowTitle,
		uint32_t _windowXPos,
		uint32_t _windowYPos,
		uint32_t _windowWidth,
		uint32_t _windowHeight,
		uint32_t _windowFlags,
		void(*_windowClosedCallback)()
	)
	{
		title = _windowTitle;
		width = _windowWidth;
		height = _windowHeight;
		closedCallback = _windowClosedCallback;
		sdlWindow = SDL_CreateWindow(title.data(), _windowXPos, _windowYPos, width, height, _windowFlags);
	}

	Window::~Window()
	{
		SDL_DestroyWindow(sdlWindow);

		{
			std::vector<Callback_Event> callbacks;
			for (auto callbk : windowCallbacks)
			{
				callbacks.push_back(callbk.first);
			}
			for (auto callbk : callbacks)
			{
				RemoveWindowCallback(callbk);
			}
		}
	}
}
