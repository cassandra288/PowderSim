#pragma once

#include <glm/glm.hpp>


namespace powd::components
{
	struct CompTransform2D
	{
	public:
		glm::vec2 position = { 0, 0 };
		float rotation = 0;
		glm::vec2 scale = { 1, 1 };
	};
}
