#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "src/Rendering/Wrappers/GlUbo.h"
#include "src/Rendering/Wrappers/GlVertexCache.h"
#include "src/Rendering/Wrappers/GlShader.h"
#include "src/Rendering/Wrappers/GlTexture.h"


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
		std::vector<rendering::GlTexture*> textures;
		rendering::GlUbo ubo;
		unsigned instanceCount = 1;
		
		PositionSpace renderSpace;

		friend rendering::SysMeshRenderer;
	};
}
