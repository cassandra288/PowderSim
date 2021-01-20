#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include "Input.h"
#include "InputDrivers.h"
#include "InputProcessor.h"


namespace powd::input
{
	class InputAction
	{
	public:
		union Data
		{
			bool _bool;
			int _int;
			float _float;
			glm::vec2 _vec2;
		};

	private:
		Data data;
		std::unordered_map<std::string, InputDriver*> drivers;
		InputDriver* leadDriver;
		std::vector<InputProcessor*> processors;

	public:
		InputAction();
		InputAction(std::string binding);
		~InputAction();
		InputAction(InputAction& _o);
		InputAction(InputAction&& _o);
		InputAction& operator =(InputAction& _o);
		InputAction& operator =(InputAction&& _o);

		template<typename T>
		T GetData();

		void AddBinding(std::string binding);
		void RemoveBinding(std::string binding);
		void ClearBindings();

		template<typename T, typename... Args>
		unsigned AddProcessor(Args... args)
		{
			unsigned i = processors.size();
			T* newProc = new T(args...);
			processors.push_back((InputProcessor*)newProc);

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

			return i;
		}
		void RemoveProcessor(unsigned i);
		void ClearProcessors();

	private:
		void OnDataChange(InputDriver* driver);
		void PickNewLeader(InputDriver* ignore = nullptr);
		void ProcessData(Data newData);


		friend void SwapData(InputAction& _l, InputAction& _r);
		friend void CopyData(InputAction& _l, InputAction& _r);
	};
}
