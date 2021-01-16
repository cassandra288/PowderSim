#pragma once

#include <string>

// This defines some boundary-crossing types. An update to this should also include an update to the DLL interface files.
#define DEF_FUNC(__ret__, __name__, ...) __ret__(*__name__)(__VA_ARGS__)

namespace powd::sand
{
	struct vec2
	{
	public:
		unsigned x;
		unsigned y;

		bool operator ==(const vec2& _o)
		{
			return x == _o.x && y == _o.y;
		}
	};

	struct exports_t
	{
	public:
		DEF_FUNC(bool, CreateNewPowder, const char*, vec2, unsigned&);
		DEF_FUNC(void, RemovePowder, unsigned);

		DEF_FUNC(unsigned, GetPowder, vec2);
		DEF_FUNC(std::string, GetPowderType, unsigned);

		DEF_FUNC(vec2, GetPowderPosition, unsigned);
		DEF_FUNC(bool, SetPowderPosition, unsigned, vec2);
		DEF_FUNC(bool, TranslatePowderPosition, unsigned, vec2);

		DEF_FUNC(void, LogMsg, const char*);
	};

	extern exports_t exports;
}

#undef DEF_FUNC
