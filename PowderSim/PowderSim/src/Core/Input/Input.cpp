#include "Input.h"

#include <unordered_map>

#include <glm/glm.hpp>


namespace powd::input
{
	std::unordered_map<std::string, InputDevice*> intern::devices;
	std::unordered_map<std::string, InputAction*> inputActions;


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
	void ClearDevices()
	{
		for (auto pair : intern::devices)
			delete pair.second;
		intern::devices.clear();
	}



	template<> bool GetInput(std::string inputName)
	{
		return inputActions[inputName]->GetData<bool>();
	}
	template<> int GetInput(std::string inputName)
	{
		return inputActions[inputName]->GetData<int>();
	}
	template<> float GetInput(std::string inputName)
	{
		return inputActions[inputName]->GetData<float>();
	}
	template<> glm::vec2 GetInput(std::string inputName)
	{
		return inputActions[inputName]->GetData<glm::vec2>();
	}


	void CreateAction(std::string name, std::string binding)
	{
		inputActions[name] = new InputAction(binding);
	}
	void DeleteAction(std::string name)
	{
		delete inputActions[name];
		inputActions.erase(name);
	}
	InputAction* GetAction(std::string name)
	{
		return inputActions[name];
	}
	void ClearActions()
	{
		for (auto pair : inputActions)
			delete pair.second;
		inputActions.clear();
	}
}
