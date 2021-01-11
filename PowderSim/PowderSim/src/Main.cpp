#include "Main.h"

#include <thread>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <CppLog/Logger.h>
#include <SOIL2/SOIL2.h>

#include "src/Core/Exceptions/RenderingExceptions.h"
#include "src/Core/Window/WindowManager.h"
#include "src/Core/Ecs/SystemProto.h"
#include "src/Core/Dispatching/CoreLoops.h"	
#include "src/Core/Utils/StringUtils.h"
#include "src/Rendering/Renderer.h"

#include "src/Core/Ecs/EntityRegistry.h"
#include "src/Core/Components/CompRenderMaterial.h"
#include "src/Core/Components/CompTransform2D.h"

#include "src/Core/Profiling/CPUProfiler.h"

USING_LOGGER


namespace powd
{
	void OnStart()
	{
		new cpplog::Logger("log.txt", "Main", 4);

		window::StartSDL();

		window::mainWindow = window::StartWindow("Powder Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN, dispatch::StopCoreLoop);
		
		rendering::StartRenderer();


		using namespace rendering;

		static const float meshData1[6] = {
			-0.5f, -0.5f,
			0.5f, -0.5f,
			0.f, 0.5f
		};
		static const float meshData2[12] = {
			-0.5f, -0.5f,
			0.5f, -0.5f,
			-0.5f, 0.5f,
			-0.5f, 0.5f,
			0.5f, -0.5f,
			0.5f, 0.5f
		};
		GlMeshID mesh = GlVertexCache::CreateMesh((void*)meshData1, 6 * sizeof(float), {}, 0);
		GlMeshID mesh2 = GlVertexCache::CreateMesh((void*)meshData2, 12 * sizeof(float), {}, 0);
		GlShader* shader = new GlShader("shader.vert", "shader.frag");
		shader->AddAttribute({ 2, GL_FLOAT, sizeof(float) });
		shader->BuildVAO();

		glm::vec3 red(1, 0, 0);
		glm::vec3 green(0, 1, 0);

		{
			auto entity1 = ecs::entities.create();
			auto& mat1 = ecs::entities.emplace<components::CompRenderMaterial>(entity1);

			mat1.mesh = mesh;
			mat1.shader = shader;
			mat1.ubo.AddData(&red);

			auto& trans = ecs::entities.emplace<components::CompTransform2D>(entity1);

			trans.position = { 0, 0 };
			trans.rotation = 0;
			trans.scale = { 1, 1 };
		}

		{
			auto entity2 = ecs::entities.create();
			auto& mat2 = ecs::entities.emplace<components::CompRenderMaterial>(entity2);

			mat2.mesh = mesh2;
			mat2.shader = shader;
			mat2.ubo.AddData(&green);

			auto& trans = ecs::entities.emplace<components::CompTransform2D>(entity2);

			trans.position = { 5, 5 };
			trans.rotation = 45;
			trans.scale = { 2, 2 };

			trans.startX = 5;
		}
	}

	class TestSystem : ecs::SystemProto
	{
		DEFINE_SYSTEM_PROTO(TestSystem);

	private:
		const float scale = 1;
		const float speed = 180.f;

	public:
		System_Tick(dt)
		{
			for (entt::entity entity : ecs::entities.view<components::CompTransform2D>())
			{
				auto& transform = ecs::entities.get<components::CompTransform2D>(entity);

				transform.position.x = transform.startX + (sin(transform.test) * scale);
				transform.test += speed * dt;
			}

			//Logger::Log(profiling::GetProfileDataStr());
		}
	};

	void OnStop()
	{
		rendering::StopRenderer();

		window::StopSDL();
	}

	

	int Main()
	{
		try
		{
			OnStart();

			auto mat = ecs::entities.get<components::CompRenderMaterial>(entt::entity(0));

			dispatch::CoreLoop();

			OnStop();
		}
		catch (const powd::exceptions::BaseException& e)
		{
			std::string msg = utils::string::ReplaceAll(e.Message(), "\n", "\n    ");
			Logger::Lock() << "Unhandled " << e.ExceptionType() << " Exception at: " << e.File() << "[" << e.Line() << "]\n    "
				<< "Message: " << msg << Logger::endl;
		}

		return 0;
	}


	IMPLEMENT_SYSTEM_PROTO(TestSystem);
}
