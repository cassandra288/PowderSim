#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include <entt.hpp>
#include <CppLog/Logger.h>

#include "InputDevice.h"


namespace powd::input
{
	class InputDevice;

	class InputDriver
	{
	private:
		std::string name;
		InputDevice* device;
		InputDriver* parent;
		std::unordered_map<std::string, InputDriver*> children;
	protected:
		entt::sigh<void(InputDriver*)> eventSignal;

	public:
		entt::sink<void(InputDriver*)> eventSink;

	public:
		InputDriver(std::string _name, InputDevice* _device, InputDriver* _parent) : name(_name), device(_device), parent(_parent), eventSink(eventSignal)
		{
			if (parent != nullptr)
				parent->children[name] = this;
			else
				device->AttachInput(name ,this);
		}
		virtual ~InputDriver()
		{
			for (auto& child : children) 
			{
				delete child.second;
			}
		}

		template<typename T>
		T ReadValue();

		template<typename T>
		void SetValue(T value);

		std::string GetName() { return name; }
		InputDevice* GetDevice() { return device; }
		InputDriver* GetParent() { return parent; }
		InputDriver* GetChild(std::string name)
		{
			if (children.find(name) == children.end())
			{
				cpplog::Logger::Log("Attempted to get non-existant child: " + name, cpplog::Logger::WARNING);
				return nullptr;
			}
			return children[name];
		}

		std::unordered_map<std::string, InputDriver*>::const_iterator begin() const { return children.begin(); }
		std::unordered_map<std::string, InputDriver*>::const_iterator end() const { return children.end(); }
		InputDriver* operator [](std::string _i) { return GetChild(_i); }

	public:
		enum class Type
		{
			_bool,
			_int,
			_float,
			_vec2
		};

	private:
		virtual void* ReadValue_Internal() = 0;
		virtual void SetValue_Internal(void* val) = 0;

	public:
		virtual Type GetType() = 0;
		std::string TypeToStr(Type _type);
	};

	class InputDriverBool : public InputDriver
	{
	public:
		bool myval;
		InputDriverBool(std::string _name, bool _val, InputDevice* _device, InputDriver* _parent = nullptr) : myval(_val), InputDriver(_name, _device, _parent) {}

	private:
		void* ReadValue_Internal() { return &myval; };
		void SetValue_Internal(void* val) { myval = *(bool*)val; eventSignal.publish(this); };
		Type GetType() { return Type::_bool; }
	};
	class InputDriverInt : public InputDriver
	{
	public:
		int myval;
		InputDriverInt(std::string _name, int _val, InputDevice* _device, InputDriver* _parent = nullptr) : myval(_val), InputDriver(_name, _device, _parent) {}

	private:
		void* ReadValue_Internal() { return &myval; };
		void SetValue_Internal(void* val) { myval = *(int*)val; eventSignal.publish(this); };
		Type GetType() { return Type::_int; }
	};
	class InputDriverFloat : public InputDriver
	{
	public:
		float myval;
		InputDriverFloat(std::string _name, float _val, InputDevice* _device, InputDriver* _parent = nullptr) : myval(_val), InputDriver(_name, _device, _parent) {}

	private:
		void* ReadValue_Internal() { return &myval; };
		void SetValue_Internal(void* val) { myval = *(float*)val; eventSignal.publish(this); };
		Type GetType() { return Type::_float; }
	};
	class InputDriverVec2 : public InputDriver
	{
	public:
		glm::vec2 myval;
		InputDriverVec2(std::string _name, glm::vec2 _val, InputDevice* _device, InputDriver* _parent = nullptr) : myval(_val), InputDriver(_name, _device, _parent) {}

	private:
		void* ReadValue_Internal() { return &myval; };
		void SetValue_Internal(void* val) { myval = *(glm::vec2*)val; eventSignal.publish(this); };
		Type GetType() { return Type::_vec2; }
	};
}
