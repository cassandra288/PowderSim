#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <GL/glew.h>
#include <CppLog/Logger.h>

#include "src/Core/Exceptions/RenderingExceptions.h"
#include "src/Core/Window/WindowManager.h"
#include "src/Core/Ecs/EntityRegistry.h"
#include "src/Core/Ecs/SystemProto.h"
#include "src/Core/Components/CompRenderMaterial.h"
#include "src/Core/Components/CompTransform2D.h"
#include "src/Core/Profiling/CPUProfiler.h"

USING_LOGGER
using namespace powd::components;


namespace powd::rendering
{
	namespace
	{
		SDL_GLContext glContext;

		struct RenderData
		{
			GlUbo coreRenderData;
		};
		RenderData* renderData;

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

	class SysMeshRenderer : ecs::SystemProto
	{
		DEFINE_SYSTEM_PROTO(SysMeshRenderer);

	public:
		System_Render(dt)
		{
			RenderFrame();
		}


	private:
		void RenderFrame()
		{
			glDepthMask(true);
			glClearColor(0.f, 0.f, 0.f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			GlVertexCache::RebindBuffers();
			renderData->coreRenderData.Bind(0);
			
			glDepthFunc(GL_LESS);
			for (auto entity : renderableView)
			{
				RenderObject(ecs::entities.get<CompRenderMaterial>(entity));
			}

			profiling::PauseSection();
			SDL_GL_SwapWindow(window::GetInstance(window::mainWindow)->getWindow());
			profiling::UnpauseSection();
			glFlush();
		}

		void GetCoreObjectData(CompRenderMaterial& _material)
		{
			if (_material.coreUbo.GetUBODataCount() != 1)
			{
				_material.coreUbo.SetUBOSize(1);
			}

			glm::mat3 modelMat(1.0f);

			CompTransform2D* transform = ecs::entities.try_get<CompTransform2D>(entt::to_entity(ecs::entities, _material));
			if (transform == nullptr)
			{
				modelMat = glm::translate(modelMat, { 0, 0 });
				modelMat = glm::rotate(modelMat, 0.f);
				modelMat = glm::scale(modelMat, { 1, 1 });
			}
			else
			{
				modelMat = glm::translate(modelMat, transform->position);
				modelMat = glm::rotate(modelMat, transform->rotation);
				modelMat = glm::scale(modelMat, transform->scale);
			}

			_material.coreUbo.ModifyData(0, &modelMat);
		}

		void RenderObject(CompRenderMaterial& _material)
		{
			GetCoreObjectData(_material);

			_material.shader->UseProgram();

			for (GlTexture* _texture : _material.textures)
			{
				_texture->Bind();
			}

			_material.coreUbo.Bind(2);
			_material.ubo.Bind(3);

			glDrawArraysInstanced(GL_TRIANGLES, GlVertexCache::GetVertexOffset(_material.mesh) / _material.shader->GetVAOStride(), GlVertexCache::GetVertexSize(_material.mesh) / _material.shader->GetVAOStride(), _material.instanceCount);
		}
	};
	IMPLEMENT_SYSTEM_PROTO(SysMeshRenderer);



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
		window::GetInstance(window::mainWindow)->AddWindowCallback([](SDL_Event e, void* _data) {
			glViewport(0, 0, window::GetInstance(window::mainWindow)->getWidth(), window::GetInstance(window::mainWindow)->getHeight());
		});

		GlVertexCache::Setup();

		renderData = new RenderData();

		glm::vec2 viewportSize(640, 360);
		renderData->coreRenderData.AddData(&viewportSize);
	}

	void StopRenderer()
	{
		SDL_GL_DeleteContext(glContext);
	}
}
