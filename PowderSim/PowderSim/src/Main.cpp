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
	rendering::GlTexture2D* texture;

	void OnStart()
	{
		new cpplog::Logger("log.txt", "Main", 4);

		window::StartSDL();

		window::mainWindow = window::StartWindow("Powder Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN, dispatch::StopCoreLoop);
		
		rendering::StartRenderer();


		using namespace rendering;
		{
			static const float meshData[24] = {
				-0.5f, -0.5f, 0.f, 0.f,
				0.5f, -0.5f, 1.f, 0.f,
				-0.5f, 0.5f, 0.f, 1.f,
				-0.5f, 0.5f, 0.f, 1.f,
				0.5f, -0.5f, 1.f, 0.f,
				0.5f, 0.5f, 1.f, 1.f
			};
			GlMeshID mesh = GlVertexCache::CreateMesh((void*)meshData, 24 * sizeof(float), {}, 0);
			GlShader* shader = new GlShader("powder_texture.vert", "powder_texture.frag");
			shader->AddAttribute({ 2, GL_FLOAT, sizeof(float) });
			shader->AddAttribute({ 2, GL_FLOAT, sizeof(float) });
			shader->BuildVAO();

			texture = new GlTexture2D(0, GlTextureFormat::RGB, 128, 72, false);
			texture->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			texture->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			texture->Bind();

			texture->Draw({ 0, 0 }, { 1, 3 }, { 255, 0, 0, 0, 255, 0, 0, 0, 255 });

			unsigned char data[3];
			texture->Read({ 0, 1 }, { 1, 1 }, data);
			Logger::Lock() << (int)data[0] << ", " << (int)data[1] << ", " << (int)data[2] << Logger::endl;

			auto entity = ecs::entities.create();
			auto& mat = ecs::entities.emplace<components::CompRenderMaterial>(entity);

			mat.mesh = mesh;
			mat.shader = shader;
			mat.textures.push_back(texture);

			auto& trans = ecs::entities.emplace<components::CompTransform2D>(entity);

			trans.position = { 0, 0 };
			trans.rotation = 0;
			trans.scale = { 1280, 720 };
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
			/*for (entt::entity entity : ecs::entities.view<components::CompTransform2D>())
			{
				auto& transform = ecs::entities.get<components::CompTransform2D>(entity);

				transform.position.x = transform.startX + (sin(transform.test) * scale);
				transform.test += speed * dt;
			}*/

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
