#include "Input.h"

#include <unordered_map>

#include <glm/glm.hpp>


namespace powd::input
{
	std::unordered_map<std::string, InputDevice*> intern::devices;


	InputDevice* GetDevice(std::string name)
	{
		if (name.find(':') == std::string::npos)
			name += ":0";

		if (intern::devices.find(name) == intern::devices.end())
			return nullptr;
		return intern::devices.find(name)->second;
	}
	InputDevice* GetDevice(std::string name, unsigned index)
	{
		return GetDevice(name + ":" + std::to_string(index));
	}
	void RemoveDevice(std::string name)
	{
		if (name.find(':') == std::string::npos)
			name += ":0";

		if (intern::devices.find(name) == intern::devices.end())
			return;
		delete intern::devices[name];
		intern::devices.erase(name);
	}
	void RemoveDevice(std::string name, unsigned index)
	{
		RemoveDevice(name + ":" + std::to_string(index));
	}



	template<> bool GetInput(std::string inputName)
	{
		return true;
	}
	template<> int GetInput(std::string inputName)
	{
		return 0;
	}
	template<> float GetInput(std::string inputName)
	{
		return 0;
	}
	template<> glm::vec2 GetInput(std::string inputName)
	{
		return { 0, 0 };
	}
}
