#include "Widget.h"

#include <cinttypes>


namespace powd::ui
{
	std::list<Widget> Widget::existing2DWidgets;
	std::list<Widget> Widget::existing3DWidgets;


	glm::vec3 Widget::PositionRawToRefined(glm::vec3 posIn)
	{
		if (!posDirty)
			return refinedPosition;

		glm::vec3 parentPos = { 0, 0, 0 };
		if (parent != nullptr)
		{
			parentPos = parent->PositionRawToRefined(parent->position);
		}
		
	}
	glm::vec3 Widget::ScaleRawToRefined(glm::vec3 scaleIn)
	{

	}
}
