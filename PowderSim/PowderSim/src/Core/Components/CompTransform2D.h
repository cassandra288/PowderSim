#pragma once

#include <glm/glm.hpp>


namespace powd::components
{
	struct CompTransform2D
	{
	public:
		glm::vec2 position;
		float rotation;
		glm::vec2 scale;

		unsigned test;
		float startX;
	};
}
