#pragma once

#include <string>

#include "InputDevice.h"
#include "InputAction.h"


namespace powd::input
{
	class InputAction;
	namespace intern
	{
		extern std::unordered_map<std::string, InputDevice*> devices;
	}

	InputDevice* GetDevice(std::string name);
	InputDevice* GetDevice(std::string name, unsigned index);
	void RemoveDevice(std::string name);
	void RemoveDevice(std::string name, unsigned index);
	void ClearDevices();

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


	/// <summary>
	/// Gets the value from a given input action made through CreateAction
	/// </summary>
	template<typename T>
	T GetInput(std::string inputName);

	void CreateAction(std::string name, std::string binding);
	void DeleteAction(std::string name);
	InputAction* GetAction(std::string name);
	void ClearActions();
}
