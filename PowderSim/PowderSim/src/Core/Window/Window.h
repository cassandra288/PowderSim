#pragma once

#include <cstdint>
#include <string>
#include <mutex>

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

		std::mutex mut;


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

		void SetMinimized(bool _val) { std::lock_guard<std::mutex>lck(mut); minimized = _val; }
		void SetWidthAndHeightInternal(uint32_t _width, uint32_t _height) { std::lock_guard<std::mutex>lck(mut); width = _width, height = _height; }

	public:
		SDL_Window* getWindow() { std::lock_guard<std::mutex>lck(mut); return sdlWindow; }
		std::string getTitle() { std::lock_guard<std::mutex>lck(mut); return title; }
		unsigned int getWidth() { std::lock_guard<std::mutex>lck(mut); return width; }
		unsigned int getHeight() { std::lock_guard<std::mutex>lck(mut); return height; }

		operator SDL_Window* () const { return sdlWindow; }

		void setTitle(std::string _title) { std::lock_guard<std::mutex>lck(mut); title = _title; SDL_SetWindowTitle(sdlWindow, title.data()); }
		void setWidth(unsigned int _width) { std::lock_guard<std::mutex>lck(mut); width = _width; SDL_SetWindowSize(sdlWindow, width, height); }
		void setHeight(unsigned int _height) { std::lock_guard<std::mutex>lck(mut); height = _height; SDL_SetWindowSize(sdlWindow, width, height); }
		void setWidthAndHeight(unsigned int _width, unsigned int _height) { std::lock_guard<std::mutex>lck(mut); width = _width; height = _height; SDL_SetWindowSize(sdlWindow, width, height); }

		uint32_t GetID() { std::lock_guard<std::mutex>lck(mut); return SDL_GetWindowID(sdlWindow); }
		bool IsMinimized() { std::lock_guard<std::mutex>lck(mut);  minimized; }


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
