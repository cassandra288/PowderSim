#include "BehaviourInterface.h"

#include "BehaviourFunctionExports.h"

#include "src/SandEngine/SandEngine.h"

#include <CppLog/Logger.h>


#define SET_FUNC(__name__) exports.##__name__ = Exp_##__name__
#define DEF_FUNC(__ret__, __name__, ...) __ret__ Exp_##__name__(__VA_ARGS__)

namespace powd::sand
{
	namespace
	{
		DEF_FUNC(bool, CreateNewPowder, const char* id, vec2 pos, unsigned& out)
		{
			return CreateNewPowder(id, { pos.x, pos.y }, out);
		}
		DEF_FUNC(void, RemovePowder, unsigned powder)
		{
			RemovePowder(powder);
		}

		unsigned Exp_GetPowder(vec2 pos)
		{
			return GetPowder({ pos.x, pos.y });
		}
		std::string Exp_GetPowderType(unsigned powder)
		{
			return GetPowderType(powder);
		}

		vec2 Exp_GetPowderPosition(unsigned powder)
		{
			glm::uvec2 pos = GetPowderPos(powder);
			return { pos.x, pos.y };
		}
		bool Exp_SetPowderPosition(unsigned powder, vec2 newPos)
		{
			return SetPowderPos({ newPos.x, newPos.y }, powder);
		}
		bool Exp_TranslatePowderPosition(unsigned powder, vec2 translation)
		{
			return TranslatePowderPos({ translation.x, translation.y }, powder);
		}

		DEF_FUNC(void, LogMsg, const char* msg)
		{
			std::string newmsg(msg);
			cpplog::Logger::Log(newmsg, cpplog::Logger::DEBUG);
		}
	}


	void Setup()
	{
		SET_FUNC(CreateNewPowder);
		SET_FUNC(RemovePowder);

		SET_FUNC(GetPowder);
		SET_FUNC(GetPowderType);

		SET_FUNC(GetPowderPosition);
		SET_FUNC(SetPowderPosition);
		SET_FUNC(TranslatePowderPosition);

		SET_FUNC(LogMsg);
	}
}
