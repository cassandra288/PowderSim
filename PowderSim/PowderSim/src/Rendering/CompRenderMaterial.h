#pragma once

#include <glm/glm.hpp>

#include "Wrappers/GlUbo.h"
#include "Wrappers/GlVertexCache.h"
#include "Wrappers/GlShader.h"


namespace powd::rendering
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
		GlUbo coreUbo;

	public:
		GlMeshID mesh;
		GlShader* shader;
		GlUbo ubo;
		
		glm::vec2 renderPos;
		PositionSpace renderSpace;

		friend void RenderObject(CompRenderMaterial& material);
	};
}
