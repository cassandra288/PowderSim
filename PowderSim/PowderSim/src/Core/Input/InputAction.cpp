#include "InputAction.h"

#include <numeric>
#include <glm/gtx/norm.hpp>

#include "src/Core/Utils/StringUtils.h"
#include "src/Core/Exceptions/GenericExceptions.h"


namespace powd::input
{
	void SwapData(InputAction& _l, InputAction& _r)
	{
		std::swap(_l.data, _r.data);
	}
	void CopyData(InputAction& _l, InputAction& _r)
	{
		_l.data = _r.data;
	}


	InputAction::InputAction()
	{

	}
	InputAction::InputAction(std::string binding)
	{
		AddBinding(binding);
	}
	InputAction::~InputAction()
	{
		ClearBindings();
		ClearProcessors();
	}
	InputAction::InputAction(InputAction& _o)
	{
		CopyData(*this, _o);
	}
	InputAction::InputAction(InputAction&& _o)
	{
		SwapData(*this, _o);
	}
	InputAction& InputAction::operator =(InputAction& _o)
	{
		CopyData(*this, _o);
		return *this;
	}
	InputAction& InputAction::operator =(InputAction&& _o)
	{
		SwapData(*this, _o);  
		return *this;
	}

	template<> bool InputAction::GetData()
	{
		return data._bool;
	}
	template<> int InputAction::GetData()
	{
		return data._int;
	}
	template<> float InputAction::GetData()
	{
		return data._float;
	}
	template<> glm::vec2 InputAction::GetData()
	{
		return data._vec2;
	}

	void InputAction::AddBinding(std::string binding)
	{
		if (drivers.find(binding) != drivers.end())
			return;

		std::vector<std::string> individals = utils::string::SplitByDelimiter(binding, ";");

		for (std::string bindStatement : individals)
		{
			if (bindStatement == "")
				continue;
			if (drivers.find(bindStatement) != drivers.end())
				continue;
			bindStatement = utils::string::RemoveTrailing(bindStatement);
			std::vector<std::string> parts = utils::string::SplitByDelimiter(bindStatement, "/");
			
			InputDevice* device = GetDevice(parts[0]);
			if (device != nullptr && parts.size() >= 2)
			{
				InputDriver* driver = (*device)[parts[1]];
				unsigned j = 1;
				for (unsigned i = 2; i < parts.size(); i++)
				{
					j = i;
					driver = (*driver)[parts[i]];
					if (driver == nullptr)
						break;
				}

				if (driver != nullptr)
				{
					if (leadDriver == nullptr)
						leadDriver = driver;
					
					if (driver->GetType() != leadDriver->GetType())
						throw exceptions::GenericException("Attempted to bind mis-matched driver type to input action.", __FILE__, __LINE__);

					driver->eventSink.connect<&InputAction::OnDataChange>(this);
					drivers[bindStatement] = driver;
				}
				else
				{
					std::string tmp = parts[0];
					for (unsigned i = 1; i <= j; i++)
					{
						tmp += "/" + parts[i];
					}
					throw exceptions::GenericException("Failed to parse binding string. Driver does not exist: " + tmp, __FILE__, __LINE__);
				}
			}
			else
			{
				throw exceptions::GenericException("Failed to parse binding string. Device does not exist: " + parts[0], __FILE__, __LINE__);
			}
		}

		PickNewLeader();
		Data tmpData;
		switch (leadDriver->GetType())
		{
		case InputDriver::Type::_bool:
			tmpData._bool = leadDriver->ReadValue<bool>();
			break;
		case InputDriver::Type::_int:
			tmpData._int = leadDriver->ReadValue<int>();
			break;
		case InputDriver::Type::_float:
			tmpData._float = leadDriver->ReadValue<float>();
			break;
		case InputDriver::Type::_vec2:
			tmpData._vec2 = leadDriver->ReadValue<glm::vec2>();
			break;
		}
		ProcessData(tmpData);
	}
	void InputAction::RemoveBinding(std::string binding)
	{
		std::vector<std::string> individals = utils::string::SplitByDelimiter(binding, ";");

		for (std::string bindStatement : individals)
		{
			if (bindStatement == "")
				continue;
			if (drivers.find(bindStatement) == drivers.end())
				return;

			drivers[bindStatement]->eventSink.disconnect<&InputAction::OnDataChange>(this);

			if (drivers[bindStatement] == leadDriver)
			{
				PickNewLeader(leadDriver);
			}


			drivers.erase(bindStatement);
		}
	}
	void InputAction::ClearBindings()
	{
		for (const auto& driverPair : drivers)
		{
			driverPair.second->eventSink.disconnect<&InputAction::OnDataChange>(this);
		}

		drivers.clear();

		leadDriver = nullptr;
	}

	void InputAction::RemoveProcessor(unsigned i)
	{
		delete processors[i];
		processors.erase(processors.begin() + i);
	}
	void InputAction::ClearProcessors()
	{
		for (unsigned i = 0; i < processors.size(); i++)
		{
			delete processors[i];
		}
		processors.clear();
	}

	void InputAction::OnDataChange(InputDriver* driver)
	{
		Data tmpData;
		switch (driver->GetType())
		{
		case InputDriver::Type::_bool:
			tmpData._bool = driver->ReadValue<bool>();
			if (leadDriver == driver)
			{
				PickNewLeader();
			}
			else if (tmpData._bool)
			{
				leadDriver = driver;
				ProcessData(tmpData);
			}
			break;
		case InputDriver::Type::_int:
			tmpData._int = driver->ReadValue<int>();
			if (leadDriver == driver)
			{
				PickNewLeader();
			}
			else if (tmpData._int > leadDriver->ReadValue<int>())
			{
				leadDriver = driver;
				ProcessData(tmpData);
			}
			break;
		case InputDriver::Type::_float:
			tmpData._float = driver->ReadValue<float>();
			if (leadDriver == driver)
			{
				PickNewLeader();
			}
			else if (tmpData._float > leadDriver->ReadValue<float>())
			{
				leadDriver = driver;
				ProcessData(tmpData);
			}
			break;
		case InputDriver::Type::_vec2:
			tmpData._vec2 = driver->ReadValue<glm::vec2>();
			if (leadDriver == driver)
			{
				PickNewLeader();
			}
			else if (glm::length2(tmpData._vec2) > glm::length2(leadDriver->ReadValue<glm::vec2>()))
			{
				leadDriver = driver;
				ProcessData(tmpData);
			}
			break;
		}
	}
	void InputAction::PickNewLeader(InputDriver* ignore)
	{
		Data tmpData;
		InputDriver* tmpLead = nullptr;
		switch (leadDriver->GetType())
		{
		case InputDriver::Type::_bool:
			tmpData._bool = false;
			break;
		case InputDriver::Type::_int:
			tmpData._int = 0;
			break;
		case InputDriver::Type::_float:
			tmpData._float = 0;
			break;
		case InputDriver::Type::_vec2:
			tmpData._vec2 = { 0, 0 };
			break;
		}

		for (const auto& driverPair : drivers)
		{
			if (driverPair.second == ignore)
				continue;

			bool breakout = false;
			Data magnitude;
			switch (driverPair.second->GetType())
			{
			case InputDriver::Type::_bool:
				tmpData._bool = driverPair.second->ReadValue<bool>();
				if (tmpData._bool)
				{
					tmpLead = driverPair.second;
					breakout = true;
				}
				break;
			case InputDriver::Type::_int:
				magnitude._int = driverPair.second->ReadValue<int>();
				if (magnitude._int < 0)
					magnitude._int *= -1;
				if (magnitude._int >= tmpData._int)
				{
					tmpData._int = magnitude._int;
					tmpLead = driverPair.second;
				}
				break;
			case InputDriver::Type::_float:
				magnitude._float = driverPair.second->ReadValue<float>();
				if (magnitude._float < 0)
					magnitude._float *= -1;
				if (magnitude._float >= tmpData._float)
				{
					tmpData._float = magnitude._float;
					tmpLead = driverPair.second;
				}
				break;
			case InputDriver::Type::_vec2:
				glm::vec2 vectr = driverPair.second->ReadValue<glm::vec2>();
				if (glm::length2(vectr) >= glm::length2(tmpData._vec2))
				{
					tmpData._vec2 = vectr;
					tmpLead = driverPair.second;
				}
				break;
			}

			if (breakout)
				break;
		}
		if (tmpLead == nullptr && (*drivers.begin()).second->GetType() == InputDriver::Type::_bool)
			tmpLead = (*drivers.begin()).second;
		leadDriver = tmpLead;
		ProcessData(tmpData);
	}
	void InputAction::ProcessData(Data newData)
	{
		switch ((*drivers.begin()).second->GetType())
		{
		case InputDriver::Type::_bool:
			for (auto processor : processors)
				newData._bool = processor->ProcessBool(newData._bool);
			if (data._bool != newData._bool)
			{
				data = newData;
				onValueChangeSign.publish(this);
			}
			break;
		case InputDriver::Type::_int:
			for (auto processor : processors)
				newData._int = processor->ProcessInt(newData._int);
			if (data._int != newData._int)
			{
				data = newData;
				onValueChangeSign.publish(this);
			}
			break;
		case InputDriver::Type::_float:
			for (auto processor : processors)
				newData._float = processor->ProcessFloat(newData._float);
			if (data._float != newData._float)
			{
				data = newData;
				onValueChangeSign.publish(this);
			}
			break;
		case InputDriver::Type::_vec2:
			for (auto processor : processors)
				newData._vec2 = processor->ProcessVec2(newData._vec2);
			if (data._vec2 != newData._vec2)
			{
				data = newData;
				onValueChangeSign.publish(this);
			}
			break;
		}
	}
}
