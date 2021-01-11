#pragma once

#include <glm/glm.hpp>

#include "src/Rendering/Wrappers/GlUbo.h"
#include "src/Rendering/Wrappers/GlVertexCache.h"
#include "src/Rendering/Wrappers/GlShader.h"


namespace powd::rendering
{
	class SysMeshRenderer;
}

namespace powd::components
{

	struct CompRenderMaterial
	{
	public:
		enum PositionSpace
		{
			World,
			Screen
		};


	private:
		rendering::GlUbo coreUbo;

	public:
		rendering::GlMeshID mesh;
		rendering::GlShader* shader;
		rendering::GlUbo ubo;
		unsigned instanceCount = 1;
		
		PositionSpace renderSpace;

		friend rendering::SysMeshRenderer;
	};
}
