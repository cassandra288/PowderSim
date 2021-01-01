#pragma once

#include "BaseException.h"


namespace powd::exceptions
{
	class GenericException : public BaseException
	{
	protected:
		std::string message;
		std::string type;

	public:
		GenericException(std::string _message, const char* _file, unsigned int _line, std::string _type = "Generic") : BaseException(_file, _line)
		{
			message = _message;
			type = _type;
		}

		std::string ExceptionType() const override
		{
			return type;
		}

		std::string Message() const override
		{
			return message;
		}
	};
}
