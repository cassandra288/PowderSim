#pragma once

#include "SystemProto.h"


namespace powd::ecs::system
{
	void RunSystems_Start()
	{
		for (std::pair<std::string, SystemProto*> proto : SystemProto::systems)
		{
			proto.second->Start();
		}
	}

	void RunSystems_End()
	{
		for (std::pair<std::string, SystemProto*> proto : SystemProto::systems)
		{
			proto.second->End();
		}
	}

	void RunSystems_PreRender(float dt)
	{
		for (std::pair<std::string, SystemProto*> proto : SystemProto::systems)
		{
			proto.second->PreRender(dt);
		}
	}

	void RunSystems_Render(float dt)
	{
		for (std::pair < std::string, SystemProto*> proto : SystemProto::systems)
		{
			proto.second->Render(dt);
		}
	}
	void RunSystems_PostRender(float dt)
	{
		for (std::pair < std::string, SystemProto*> proto : SystemProto::systems)
		{
			proto.second->PostRender(dt);
		}
	}

	void RunSystems_PreTick(float dt)
	{
		for (std::pair < std::string, SystemProto*> proto : SystemProto::systems)
		{
			proto.second->PreTick(dt);
		}
	}

	void RunSystems_Tick(float dt)
	{
		for (std::pair < std::string, SystemProto*> proto : SystemProto::systems)
		{
			proto.second->Tick(dt);
		}
	}

	void RunSystems_PostTick(float dt)
	{
		for (std::pair < std::string, SystemProto*> proto : SystemProto::systems)
		{
			proto.second->PostTick(dt);
		}
	}
}
