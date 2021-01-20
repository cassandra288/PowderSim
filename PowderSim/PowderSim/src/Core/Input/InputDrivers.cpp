#include "InputDrivers.h"


namespace powd::input
{
	std::string InputDriver::TypeToStr(Type _type)
	{
		switch (_type)
		{
		case Type::_bool:
			return "Bool";
		case Type::_int:
			return "Int";
		case Type::_float:
			return "Float";
		case Type::_vec2:
			return "Vec2";
		default:
			return "Invalid";
		}
	}


	template<> bool InputDriver::ReadValue()
	{
		if (GetType() != Type::_bool)
		{
			cpplog::Logger::Log("Attempted to read type Bool on an InputDriver of type: " + TypeToStr(GetType()), cpplog::Logger::WARNING);
			return false;
		}
		return *(bool*)ReadValue_Internal();
	}
	template<> int InputDriver::ReadValue()
	{
		if (GetType() != Type::_int)
		{
			cpplog::Logger::Log("Attempted to read type Int on an InputDriver of type: " + TypeToStr(GetType()), cpplog::Logger::WARNING);
			return 0;
		}
		return *(int*)ReadValue_Internal();
	}
	template<> float InputDriver::ReadValue()
	{
		if (GetType() != Type::_float)
		{
			cpplog::Logger::Log("Attempted to read type Float on an InputDriver of type: " + TypeToStr(GetType()), cpplog::Logger::WARNING);
			return 0.f;
		}
		return *(float*)ReadValue_Internal();
	}
	template<> glm::vec2 InputDriver::ReadValue()
	{
		if (GetType() != Type::_vec2)
		{
			cpplog::Logger::Log("Attempted to read type Vec2 on an InputDriver of type: " + TypeToStr(GetType()), cpplog::Logger::WARNING);
			return { 0, 0 };
		}
		return *(glm::vec2*)ReadValue_Internal();
	}

	template<> void InputDriver::SetValue(bool val)
	{
		if (GetType() != Type::_bool)
			cpplog::Logger::Log("Attempted to set type Bool on an InputDriver of type: " + TypeToStr(GetType()), cpplog::Logger::WARNING);
		else
			SetValue_Internal(&val);
	}
	template<> void InputDriver::SetValue(int val)
	{
		if (GetType() != Type::_int)
			cpplog::Logger::Log("Attempted to set type Int on an InputDriver of type: " + TypeToStr(GetType()), cpplog::Logger::WARNING);
		else
			SetValue_Internal(&val);
	}
	template<> void InputDriver::SetValue(float val)
	{
		if (GetType() != Type::_float)
			cpplog::Logger::Log("Attempted to set type Float on an InputDriver of type: " + TypeToStr(GetType()), cpplog::Logger::WARNING);
		else
			SetValue_Internal(&val);
	}
	template<> void InputDriver::SetValue(glm::vec2 val)
	{
		if (GetType() != Type::_vec2)
			cpplog::Logger::Log("Attempted to set type Vec2 on an InputDriver of type: " + TypeToStr(GetType()), cpplog::Logger::WARNING);
		else
			SetValue_Internal(&val);
	}


	void InputDriverVec2::SetValue_Internal(void* val)
	{
		myval = *(glm::vec2*)val;
		eventSignal.publish(this);
		children["x"]->SetValue<float>(1.f);
		children["y"]->SetValue<float>(1.f);
	}
}
