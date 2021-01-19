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

#include "src/Core/Input/InputDrivers.h"
#include "src/Core/Input/InputAction.h"
#include "src/Core/Input/Input.h"

USING_LOGGER


namespace powd
{	
	input::InputAction* action;

	class TmpProcessor : input::InputProcessor
	{
	public:
		bool ProcessBool(bool val) { return !val; }
	};

	void OnStart()
	{
		new cpplog::Logger("log.txt", "Main", 4);

		window::StartSDL();

		window::mainWindow = window::StartWindow("Powder Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN, dispatch::StopCoreLoop);
		
		rendering::StartRenderer();

		{
			using namespace input;

			AddDevice<InputDevice_Keyboard, std::string>("keyboard", "Keyboard");

			action = new InputAction("keyboard/F;keyboard/G;keyboard/H;keyboard/J");
			action->AddProcessor<TmpProcessor>();
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
			//Logger::Log(std::to_string(action->GetData<bool>()));
			Logger::Log(profiling::GetProfileDataStr());
		}
	};

	void OnStop()
	{
		input::RemoveDevice("keyboard");

		rendering::StopRenderer();

		window::StopSDL();
	}

	

	int Main()
	{
		try
		{
			OnStart();

			dispatch::CoreLoop();

			OnStop();
		}
		catch (const powd::exceptions::BaseException& e)
		{
			std::string msg = utils::string::ReplaceAll(e.Message(), "\n", "\n    ");
			Logger::Lock(Logger::ERROR) << "Unhandled " << e.ExceptionType() << " Exception at: " << e.File() << "[" << e.Line() << "]\n    "
				<< "Message: " << msg << Logger::endl;
		}

		return 0;
	}


	IMPLEMENT_SYSTEM_PROTO(TestSystem);
}
