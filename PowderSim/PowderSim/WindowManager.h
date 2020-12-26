#pragma once

#include <cstdint>

#include "Window.h"


namespace powd::window
{
	extern uint32_t mainWindow;

	
	/// <summary>
	/// Gets a window instance from its ID.
	/// </summary>
	/// <returns>Instance returned from the ID or nullptr if ID doesn't exist.</returns>
	Window* GetInstance(uint32_t _id);

	/// <summary>
	/// Open a new window.
	/// </summary>
	/// <param name="_windowFlags">Refer to SDL documentation on window flags.</param>
	/// <param name="_windowClosedCallback">Optional callback to be called when the window is closed.</param>
	/// <returns>ID of the new window.</returns>
	uint32_t StartWindow(std::string _windowTitle = "",
		uint32_t _windowXPos = 0,
		uint32_t _windowYPos = 0,
		uint32_t _windowWidth = 0,
		uint32_t _windowHeight = 0,
		uint32_t _windowFlags = 0,
		void(*_windowClosedCallback)() = {}
	);

	/// <summary>
	/// Stops a window.
	/// </summary>
	void StopWindow(uint32_t _id);

	/// <summary>
	/// Starts SDL.
	/// </summary>
	/// <param name="_flags">Refer to the SDL documentation on SDL_INIT flags</param>
	void StartSDL(uint32_t _flags = 0);

	/// <summary>
	/// Stops SDL.
	/// </summary>
	void StopSDL();

	/// <summary>
	/// Flushes the SDL event queue. It is not guaranteed to hit all the events as it has a max event counter.
	/// </summary>
	/// <returns>Returns the number of unprocessed events.</returns>
	int FlushEvents();
}
