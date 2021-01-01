#include "Renderer.h"

#include <GL/glew.h>
#include <CppLog/Logger.h>

#include "src/Core/Exceptions/RenderingExceptions.h"
#include "src/Core/Window/WindowManager.h"
#include "src/Core/Ecs/EntityRegistry.h"
#include "CompRenderMaterial.h"

USING_LOGGER
//TODO: GlTexture


namespace powd::rendering
{
	namespace
	{
		SDL_GLContext glContext;

		auto renderableView = ecs::entities.view<CompRenderMaterial>(); // TODO: Implement some UI component that can help diffrentiate between world and screenspace rendering


		void GLAPIENTRY OpenGLDebugCallback(GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam)
		{
			std::string msg;
			msg = std::string("[") + std::to_string(id) + std::string("] ") + std::string(message);

			switch (severity)
			{
			case GL_DEBUG_SEVERITY_HIGH:
				Logger::Log(msg, 100);
				break;
			case GL_DEBUG_SEVERITY_MEDIUM:
			case GL_DEBUG_SEVERITY_LOW:
				Logger::Log(msg, 101);
				break;
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				Logger::Log(msg, 102);
				break;
			}
		}
	}


	void RenderObject(CompRenderMaterial& material)
	{
		material.shader->UseProgram();

		material.coreUbo.Bind(2);
		material.ubo.Bind(3);

		glDrawArrays(GL_TRIANGLES, GlVertexCache::GetVertexOffset(material.mesh) / material.shader->GetVAOStride(), GlVertexCache::GetVertexSize(material.mesh) / material.shader->GetVAOStride());
	}


	void StartRenderer()
	{
		glContext = SDL_GL_CreateContext(window::GetInstance(window::mainWindow)->getWindow());

		if (glewInit() != GLEW_OK)
			throw exceptions::GlException(std::string("GlewInit has failed."), glGetError(), __FILE__, __LINE__);	

		Logger::RegisterNewTag(100, "GlError", 1, Logger::COLOR_BRIGHT_RED, Logger::COLOR_BLACK, "renderlog.txt");
		Logger::RegisterNewTag(101, "GlWarning", 3, Logger::COLOR_BRIGHT_YELLOW, Logger::COLOR_BLACK, "renderlog.txt");
		Logger::RegisterNewTag(102, "GlInfo", 5, Logger::COLOR_WHITE, Logger::COLOR_BLACK, "renderlog.txt");

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLDebugCallback, nullptr);

		glViewport(0, 0, window::GetInstance(window::mainWindow)->getWidth(), window::GetInstance(window::mainWindow)->getHeight());

		GlVertexCache::Setup();
	}

	void RenderFrame()
	{
		glDepthMask(true);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		GlVertexCache::RebindBuffers();

		glDepthFunc(GL_LESS);
		for (auto entity : renderableView)
		{
			RenderObject(ecs::entities.get<CompRenderMaterial>(entity));
		}

		SDL_GL_SwapWindow(window::GetInstance(window::mainWindow)->getWindow());
		glFlush();
	}

	void StopRenderer()
	{
		SDL_GL_DeleteContext(glContext);
	}
}
