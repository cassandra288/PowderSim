#pragma once

#include <cstdint>
#include <string>

#include <SDL2/SDL.h>


namespace powd::window
{
	typedef void(*Callback_Close)();

	class Window
	{
	private:
		SDL_Window* sdlWindow;
		std::string title = "";
		uint32_t width = 0;
		uint32_t height = 0;
		bool minimized = false;
		Callback_Close closedCallback;


		Window(
			std::string _windowTitle = "",
			uint32_t _windowXPos = 0,
			uint32_t _windowYPos = 0,
			uint32_t _windowWidth = 0,
			uint32_t _windowHeight = 0,
			uint32_t _windowFlags = 0,
			void(*_windowClosedCallback)() = {}
		);
		~Window();

	public:
		SDL_Window* getWindow() { return sdlWindow; }
		std::string getTitle() { return title; }
		unsigned int getWidth() { return width; }
		unsigned int getHeight() { return height; }

		operator SDL_Window* () const { return sdlWindow; }

		void setTitle(std::string _title) { title = _title; SDL_SetWindowTitle(sdlWindow, title.data()); }
		void setWidth(unsigned int _width) { width = _width; SDL_SetWindowSize(sdlWindow, width, height); }
		void setHeight(unsigned int _height) { height = _height; SDL_SetWindowSize(sdlWindow, width, height); }
		void setWidthAndHeight(unsigned int _width, unsigned int _height) { width = _width; height = _height; SDL_SetWindowSize(sdlWindow, width, height); }

		uint32_t GetID() { return SDL_GetWindowID(sdlWindow); }
		bool IsMinimized() { return minimized; }


		friend uint32_t StartWindow(
			std::string _windowTitle,
			uint32_t _windowXPos,
			uint32_t _windowYPos,
			uint32_t _windowWidth,
			uint32_t _windowHeight,
			uint32_t _windowFlags,
			void(*_windowClosedCallback)()
		);
		friend void StopWindow(uint32_t _id);
		friend int FlushEvents();
	};
}
