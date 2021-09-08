#pragma once

#include <list>

#include <glm/glm.hpp>

#include "src/Core/Utils/Bitflag.h"

namespace powd::ui
{
	class Widget
	{
	public:
		enum ContainerFlags
		{
			XPosAbsolute = 0b1 << 0,
			XPosRelative = 0b1 << 1,
			YPosAbsolute = 0b1 << 2,
			YPosRelative = 0b1 << 3,

			XScaleAbsolute = 0b1 << 4,
			XScaleRelative = 0b1 << 5,
			YScaleAbsolute = 0b1 << 6,
			YScaleRelative = 0b1 << 7,

			XOriginTop =    0b1  << 8,
			XOriginBottom = 0b1  << 9,
			XOriginMiddle = 0b11 << 8,
			YOriginLeft =   0b1  << 10,
			YOriginRight =  0b1  << 11,
			YOriginMiddle = 0b11 << 10,

			NullIdentifier = 0b1 << 12
		};


	private:
		Widget* parent = nullptr;
		utils::Bitflag<unsigned int> containerFlags = 0;

		glm::vec3 refinedPosition = { 0, 0, 0 };
		bool posDirty = true;
		glm::vec3 refinedScale = { 0, 0, 0 };
		bool scaleDirty = true;

	protected:
		glm::vec3 position = { 0, 0, 0 };
		glm::vec3 scale = { 0, 0, 0 };

		glm::vec3 PositionRawToRefined(glm::vec3 posIn);
		glm::vec3 ScaleRawToRefined(glm::vec3 scaleIn);

		virtual void Draw() = 0;


	private:
		static std::list<Widget> existing2DWidgets;
		static std::list<Widget> existing3DWidgets;

	public:
		static void Draw2DFrame()
		{
			std::list<Widget>::iterator it = existing2DWidgets.begin();
			while (it != existing2DWidgets.end())
			{
				if (!(*it).containerFlags.TestMask(NullIdentifier)) // this checks if the Widget is null
				{
					(*it).Draw();

					(*it).posDirty = true;
					(*it).scaleDirty = true;
				}

				it++;
			}
		}

		static void Draw3DFrame()
		{
			std::list<Widget>::iterator it = existing3DWidgets.begin();
			while (it != existing3DWidgets.end())
			{
				if (!(*it).containerFlags.TestMask(NullIdentifier)) // this checks if the Widget is null
				{
					(*it).Draw();

					(*it).posDirty = true;
					(*it).scaleDirty = true;
				}

				it++;
			}
		}
	};
}
