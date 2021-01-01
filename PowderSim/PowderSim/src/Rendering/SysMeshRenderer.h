#pragma once

#include "src/Core/Ecs/SystemProto.h"
#include "src/Rendering/Renderer.h"


namespace powd::rendering
{
	class SysMeshRenderer : ecs::SystemProto
	{
		DEFINE_SYSTEM_PROTO(SysMeshRenderer);

	public:
		System_Render(dt)
		{
			RenderFrame();
		}
	};
}
