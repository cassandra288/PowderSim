#include "Main.h"

#include <thread>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <CppLog/Logger.h>

#include "src/Core/Exceptions/RenderingExceptions.h"
#include "src/Core/Window/WindowManager.h"
#include "src/Core/Ecs/SystemProto.h"
#include "src/Core/Dispatching/CoreLoops.h"	
#include "src/Core/Utils/StringUtils.h"
#include "src/Rendering/Renderer.h"

#include "src/Core/Ecs/EntityRegistry.h"
#include "src/Rendering/CompRenderMaterial.h"

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
			-1.f, -1.f,
			0.f, -1.f,
			-0.5f, 0.f
		};
		static const float meshData2[12] = {
			0.f, 0.f,
			1.f, 0.f,
			0.f, 1.f,
			0.f, 1.f,
			1.f, 0.f,
			1.f, 1.f
		};
		GlMeshID mesh = GlVertexCache::CreateMesh((void*)meshData1, 6 * sizeof(float), {}, 0);
		GlMeshID mesh2 = GlVertexCache::CreateMesh((void*)meshData2, 12 * sizeof(float), {}, 0);
		GlShader* shader = new GlShader("shader.vert", "shader.frag");
		shader->AddAttribute({ 2, GL_FLOAT, sizeof(float) });
		shader->BuildVAO();

		glm::vec3 red(1, 0, 0);
		glm::vec3 green(0, 1, 0);

		glm::vec3 test1(0, 1, 2);
		glm::vec3 test2[] = { {3, 4, 5}, {6, 7, 8} };
		glm::mat2x2 test3 = glm::mat2x2(9, 10, 11, 12); // 9  11
														// 10 12
		glm::mat2x2 test4[] = { {13, 14, 15, 16}, {17, 18, 19, 20} };
		float tests[] = { 21, 22, 23, 24, 25 };

		auto entity1 = ecs::entities.create();
		auto entity2 = ecs::entities.create();
		auto& mat1 = ecs::entities.emplace<rendering::CompRenderMaterial>(entity1);

		mat1.mesh = mesh;
		mat1.shader = shader;
		mat1.renderPos = glm::vec2(0, 0);
		mat1.ubo.AddData(&red);

		mat1.ubo.AddData(&test1);
		mat1.ubo.AddData(test2, 2);
		mat1.ubo.AddData(&test3);
		mat1.ubo.AddData(test4, 2);
		for (unsigned i = 0; i < 2; i++)
		{
			mat1.ubo.AddData(&tests[i]);
		}
		mat1.ubo.MarkStructEnd();
		for (unsigned i = 2; i < 4; i++)
		{
			mat1.ubo.AddData(&tests[i]);
		}
		mat1.ubo.MarkStructEnd();
		mat1.ubo.AddData(&tests[4]);
		
		auto& mat2 = ecs::entities.emplace<rendering::CompRenderMaterial>(entity2);

		mat2.mesh = mesh2;
		mat2.shader = shader;
		mat2.renderPos = glm::vec2(0, 0);
		mat2.ubo.AddData(&green);

		mat2.ubo.AddData(&test1);
		mat2.ubo.AddData(test2, 2);
		mat2.ubo.AddData(&test3);
		mat2.ubo.AddData(test4, 2);
		for (unsigned i = 0; i < 2; i++)
		{
			mat2.ubo.AddData(&tests[i]);
		}
		mat2.ubo.MarkStructEnd();
		for (unsigned i = 2; i < 4; i++)
		{
			mat2.ubo.AddData(&tests[i]);
		}
		mat2.ubo.MarkStructEnd();
		mat2.ubo.AddData(&tests[4]);
	}

	class TestSystem : ecs::SystemProto
	{
		DEFINE_SYSTEM_PROTO(TestSystem);

	public:
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

			auto mat = ecs::entities.get<rendering::CompRenderMaterial>(entt::entity(0));

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
