#include "CoreLoops.h"

#include <atomic>
#include <chrono>
#include <cstdint>

#include "src/Core/Math/CommonMath.h"
#include "src/Core/Window/WindowManager.h"
#include "src/Core/Ecs/SystemProtoFunctions.h"
#include "src/Core/Profiling/CPUProfiler.h"
#include <CppLog/Logger.h>


#define TIMEPOINT_VARS(__name__)	std::chrono::steady_clock::time_point __name__; \
									std::chrono::steady_clock::time_point __name__##Last;


namespace powd::dispatch
{
	namespace
	{
		struct TimepointData
		{
		public:
			TIMEPOINT_VARS(tick);
			TIMEPOINT_VARS(render);
		};


		std::atomic<bool> running = false;


		const uint32_t tickFrequency = 30;
		const uint32_t renderFrequency = 60;
		TimepointData timepoints;


		float GetDeltaTime(std::chrono::steady_clock::time_point _now, std::chrono::steady_clock::time_point _last)
		{
			return (float)(_now - _last).count() / (float)1000000000;
		}
	}

	void StopCoreLoop()
	{
		running = false;
	}


	void CoreLoop()
	{
		if (running)
			return;
		running = true;

		std::chrono::steady_clock::time_point nextUpdate = std::chrono::steady_clock::now();

		uint32_t cycleFrequency = math::lcm(tickFrequency, renderFrequency); // calculate LCM of all of the update frequencies.

		// iterate through all the values in timepoints and initialize them to now()
		std::chrono::steady_clock::time_point* pointsStrt = (std::chrono::steady_clock::time_point*)&timepoints;
		for (uint32_t i = 0; i < sizeof(timepoints) / sizeof(std::chrono::steady_clock::time_point); i++)
		{
			*pointsStrt = std::chrono::steady_clock::now();
			pointsStrt++;
		}

		std::chrono::nanoseconds timerIncrement(1000000000 / cycleFrequency);
		std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point currentTime;
		float dt;

		const std::chrono::milliseconds roughness(100); // thread waiting isn't precise and therefor we need some roughness to our time

		while (running)
		{
			if (std::chrono::steady_clock::now() >= nextUpdate)
			{
				profiling::intern::StartFrame();

				nextUpdate += timerIncrement;

				currentTime = std::chrono::steady_clock::now();

				profiling::StartSectionProfile("Window Event Flushing");
				window::FlushEvents();
				profiling::StopSectionProfile();

				if (!running)
					break;

				if (currentTime >= timepoints.tick - roughness)
				{
					profiling::StartSectionProfile("Tick Loop");
					while (currentTime >= timepoints.tick - roughness)
					{
						ecs::system::RunSystems_PreTick(1.f / tickFrequency);
						ecs::system::RunSystems_Tick(1.f / tickFrequency);
						ecs::system::RunSystems_PostTick(1.f / tickFrequency);

						timepoints.tick += std::chrono::nanoseconds(1000000000 / tickFrequency);
						break;
					}
					profiling::StopSectionProfile();
				}

				if (currentTime >= timepoints.render - roughness)
				{
					profiling::StartSectionProfile("Render Loop");
					dt = GetDeltaTime(currentTime, timepoints.renderLast);

					ecs::system::RunSystems_PreRender(dt);
					ecs::system::RunSystems_Render(dt);
					ecs::system::RunSystems_PostRender(dt);

					timepoints.renderLast = currentTime;
					timepoints.render = currentTime + std::chrono::nanoseconds(1000000000 / renderFrequency);
					profiling::StopSectionProfile();
				}


				lastTime = currentTime;
				std::this_thread::sleep_until(nextUpdate - roughness);

				profiling::StopSectionProfile();

				//cpplog::Logger::Log(profiling::GetProfileDataStr());
			}
		}
	}
}
