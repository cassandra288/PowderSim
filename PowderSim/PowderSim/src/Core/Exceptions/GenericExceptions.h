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

	class InvalidArg : public BaseException
	{
	protected:
		std::string message;
		unsigned char argIndex = 255;

	public:
		InvalidArg(std::string _message, const char* _file, unsigned int _line, unsigned char _argIndex = 255) : BaseException(_file, _line)
		{
			message = _message;
			argIndex = _argIndex;
		}

		std::string ExceptionType() const override
		{
			return "Invalid Arg[" + std::to_string(argIndex) + "]";
		}

		std::string Message() const override
		{
			return message;
		}
	};
}
