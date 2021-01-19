#pragma once

#include <string>

#include "InputDevice.h"


namespace powd::input
{
	namespace intern
	{
		extern std::unordered_map<std::string, InputDevice*> devices;
	}

	InputDevice* GetDevice(std::string name);
	InputDevice* GetDevice(std::string name, unsigned index);
	void RemoveDevice(std::string name);
	void RemoveDevice(std::string name, unsigned index);
	template<typename T, typename... Args>
	void AddDevice(std::string name, Args... args)
	{
		if (name.find(':') == std::string::npos)
			name += ":0";
		intern::devices.emplace(name, new T(args...));
	}
	template<typename T, typename... Args>
	void AddDevice(std::string name, unsigned index, Args... args)
	{
		AddDevice<T, Args...>(name + ":" + std::to_string(index), args...);
	}


	template<typename T>
	T GetInput(std::string inputName);
}
