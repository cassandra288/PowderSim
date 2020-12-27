#include "Renderer.h"

#include <GL/glew.h>

#include "src/Core/Exceptions/RenderingExceptions.h"
#include "src/Core/Window/WindowManager.h"


namespace powd::rendering
{
	namespace
	{
		SDL_GLContext glContext;
	}


	void StartRenderer()
	{
		glContext = SDL_GL_CreateContext(window::GetInstance(window::mainWindow)->getWindow());

		if (glewInit() != GLEW_OK)
			throw exceptions::GlewException(std::string("GlewInit has failed."), glGetError(), __FILE__, __LINE__);	
	}

	void RenderFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT);

		SDL_GL_SwapWindow(window::GetInstance(window::mainWindow)->getWindow());
	}

	void StopRenderer()
	{
		SDL_GL_DeleteContext(glContext);
	}
}
