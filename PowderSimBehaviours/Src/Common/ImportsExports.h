#pragma once
#include <limits>
#include <string>


#define EXPORT extern "C" _declspec(dllexport)
#define BEHAVIOUR_FUNCTION(__name__) EXPORT bool __name__(unsigned powder)

#define DEF_FUNC(__ret__, __name__, ...) __ret__(*__name__)(__VA_ARGS__)

constexpr unsigned nullPowder = std::numeric_limits<unsigned>::max();

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

struct imports_t
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

extern imports_t imports;

EXPORT extern const char* Setup(imports_t _imports);

#undef DEF_FUNC
