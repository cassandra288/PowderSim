#pragma once

#include <unordered_map>
#include <string>

#include <entt.hpp>


// These two macros are used to define and implement some prototype data to enable the class as a true ECS system.
#define DEFINE_SYSTEM_PROTO(__type__)	private: static powd::ecs::SystemProto* SystemProto_##__type__; \
										public: static const std::string systemUID;

#define IMPLEMENT_SYSTEM_PROTO(__type__)	powd::ecs::SystemProto* __type__##::SystemProto_##__type__ = powd::ecs::SystemProto::AddPrototype(new __type__(), #__type__); \
											const std::string __type__##::systemUID = #__type__;


#pragma region FunctionDefines
#define System_PreRender(__dt__) void PreRender(float __dt__)
#define System_Render(__dt__) void Render(float __dt__)
#define System_PostRender(__dt__) void PostRender(float __dt__)

#define System_PreTick(__dt__) void PreTick(float __dt__)
#define System_Tick(__dt__) void Tick(float __dt__)
#define System_PostTick(__dt__) void PostTick(float __dt__)
#pragma endregion

namespace powd::ecs
{
	class SystemProto
	{
	public:
		static std::unordered_map<std::string, SystemProto*> systems;

		static SystemProto* AddPrototype(SystemProto* _p, std::string _UID)
		{
			systems[_UID] = _p;
			return _p;
		}


#pragma region UpdateFunctions
		virtual System_PreRender(dt) {};
		virtual System_Render(dt) {};
		virtual System_PostRender(dt) {};

		virtual System_PreTick(dt) {};
		virtual System_Tick(dt) {};
		virtual System_PostTick(dt) {};
#pragma endregion
	};
}
