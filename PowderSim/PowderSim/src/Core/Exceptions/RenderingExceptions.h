#pragma once

#include <GL/glew.h>
#include <string>

#include "BaseException.h"


namespace powd::exceptions
{
	class GlException : public BaseException
	{
	protected:
		std::string message;
		GLenum errorCode;

	public:
		GlException(std::string _message, GLenum _errorCode, const char* _file, unsigned int _line) : BaseException(_file, _line)
		{
			message = _message;
			errorCode = _errorCode;
		}

		std::string ExceptionType() const override
		{
			return "OpenGL";
		}

		std::string Message() const override
		{
			return message + "\n    Error Code " + std::to_string(errorCode) + ": " + std::string((const char*)glewGetErrorString(errorCode));
		}
	};
}
