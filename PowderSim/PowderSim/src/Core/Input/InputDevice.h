#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>

#include <SDL2/SDL.h>
#include <CppLog/Logger.h>


namespace powd::input
{
	class InputDriver;
	namespace intern
	{
		class InputProcessor;
	}

	class InputDevice
	{
	protected:
		static std::unordered_set<InputDevice*> devices;
		std::string name;
		std::unordered_map<std::string, InputDriver*> inputs;

		InputDevice(InputDevice& _o) = delete;
		InputDevice& operator =(InputDevice& _o) = delete;

	public:
		InputDevice(std::string _name) : name(_name) { devices.insert(this); }
		InputDevice(InputDevice&& _o);
		InputDevice& operator =(InputDevice&& _o);
		virtual ~InputDevice();

		std::string GetName() { return name; }
		InputDriver* GetInput(std::string name)
		{
			if (inputs.find(name) == inputs.end())
			{
				cpplog::Logger::Log("Attempted to get non-existant input: " + name, cpplog::Logger::WARNING);
				return nullptr;
			}
			return inputs[name];
		}

		std::unordered_map<std::string, InputDriver*>::const_iterator begin() const { return inputs.begin(); }
		std::unordered_map<std::string, InputDriver*>::const_iterator end() const { return inputs.end(); }
		InputDriver* operator [](std::string _i) { return GetInput(_i); }

	private:
		void AttachInput(std::string name, InputDriver* driver) { inputs[name] = driver; }
		virtual void UpdateInput(SDL_Event& e) {};

		friend InputDriver;
		friend intern::InputProcessor;
	};

	namespace intern
	{
		class InputProcessor
		{
		public:
			static void ProcessEvent(SDL_Event& e)
			{
				for (auto& device : InputDevice::devices)
				{
					device->UpdateInput(e);
				}
			}
		};
	}


	class InputDevice_Keyboard : public InputDevice
	{
	public:
		InputDevice_Keyboard(std::string _name);
	private:
		void UpdateInput(SDL_Event& e);
	};
	class InputDevice_Mouse : public InputDevice
	{
	public:
		InputDevice_Mouse(std::string _name);
	private:
		void UpdateInput(SDL_Event& e);
	};
}
