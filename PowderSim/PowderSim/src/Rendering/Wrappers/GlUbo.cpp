#include "GlUbo.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>


namespace powd::rendering
{
	namespace
	{
		bool TestMask(char _mask1, char _mask2)
		{
			switch (_mask2)
			{
			case Float:
			case Int:
			case Uint:
				return (_mask1 & 0b110000) == _mask2;
				break;
			case Basic:
			case Vec:
			case Mat:
				return (_mask1 & 0b001100) == _mask2;
				break;
			case Single:
			case Array:
				return (_mask1 & 0b000011) == _mask2;
				break;
			default:
				return (_mask1 & _mask2) == _mask2;
			}
		}
	}


	GlUbo::GlUbo()
	{
		uboData = new std::vector<UboData*>();
		glGenBuffers(1, &UBO);

		referenceCounter = 1;
	}
	GlUbo::~GlUbo()
	{
		referenceCounter--;
		if (referenceCounter > 0) return;

		delete uboData;
		glDeleteBuffers(1, &UBO);
	}
	GlUbo::GlUbo(GlUbo& _o)
	{
		UBO = _o.UBO;
		uboData = _o.uboData;
		_o.referenceCounter++;
		referenceCounter = _o.referenceCounter;
	}
	GlUbo::GlUbo(GlUbo&& _o) noexcept
	{
		std::swap(UBO, _o.UBO);
		std::swap(uboData, _o.uboData);
		std::swap(referenceCounter, _o.referenceCounter);
	}
	GlUbo& GlUbo::operator = (GlUbo & _o)
	{
		UBO = _o.UBO;
		uboData = _o.uboData;
		_o.referenceCounter++;
		referenceCounter = _o.referenceCounter;
		return *this;
	}
	GlUbo& GlUbo::operator =(GlUbo&& _o) noexcept
	{
		std::swap(UBO, _o.UBO);
		std::swap(uboData, _o.uboData);
		std::swap(referenceCounter, _o.referenceCounter);
		return *this;
	}

	void GlUbo::ClearUBO()
	{
		SetUBOSize(0);
	}
	void GlUbo::SetUBOSize(unsigned _size)
	{
		for (unsigned i = _size; i < uboData->size(); i++)
		{
			delete (*uboData)[i];
		}
		bool bigger = false;
		unsigned oldSize = uboData->size();
		if (_size > uboData->size()) bigger = true;

		uboData->resize(_size);

		if (bigger)
		{
			for (unsigned i = oldSize; i < uboData->size(); i++)
			{
				(*uboData)[i] = new UboData();
			}
		}

		RemakeUBO();
	}
	void GlUbo::MarkStructEnd()
	{
		UboData* data = new UboData();

		data->index = uboData->size();
		data->typeMask = StructEnd;

		data->dataOffset = 0;
		if (uboData->size() > 0)
		{
			auto prevData = (*uboData)[uboData->size() - 1];
			data->dataOffset = prevData->dataOffset + prevData->dataSize;
		}

		uboData->push_back(data);

		RemakeUBO(data, nullptr);
	}

	void GlUbo::Bind(unsigned _i)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, _i, UBO);
	}

	void GlUbo::RewriteData(UboData* _data, void* _rawData)
	{
		// These two long blocks convert glm types to raw values
		if (TestMask(_data->typeMask, UboDataMask::Vec))
		{
			if (TestMask(_data->matSize, UboMatSize::Size1x2)) { _rawData = glm::value_ptr(*(glm::vec2*)_rawData); }
			else if (TestMask(_data->matSize, Size1x3)) { _rawData = glm::value_ptr(*(glm::vec3*)_rawData); }
			else { _rawData = glm::value_ptr(*(glm::vec3*)_rawData); }
		}
		else if (TestMask(_data->typeMask, UboDataMask::Mat))
		{
			if (TestMask(_data->matSize, UboMatSize::Size1x2))
			{
				if (TestMask(_data->matSize, UboMatSize::Size2x1)) { _rawData = glm::value_ptr(*(glm::mat2x2*)_rawData); }
				else if (TestMask(_data->matSize, Size3x1)){ _rawData = glm::value_ptr(*(glm::mat3x2*)_rawData); }
				else { _rawData = glm::value_ptr(*(glm::mat4x2*)_rawData); }
			}
			else if (TestMask(_data->matSize, Size1x3))
			{
				if (TestMask(_data->matSize, UboMatSize::Size2x1)) { _rawData = glm::value_ptr(*(glm::mat2x3*)_rawData); }
				else if (TestMask(_data->matSize, Size3x1)) { _rawData = glm::value_ptr(*(glm::mat3x3*)_rawData); }
				else { _rawData = glm::value_ptr(*(glm::mat4x3*)_rawData); }
			}
			else
			{
				if (TestMask(_data->matSize, UboMatSize::Size2x1)) { _rawData = glm::value_ptr(*(glm::mat2x4*)_rawData); }
				else if (TestMask(_data->matSize, Size3x1)) { _rawData = glm::value_ptr(*(glm::mat3x4*)_rawData); }
				else { _rawData = glm::value_ptr(*(glm::mat4x4*)_rawData); }
			}
		}

		glBindBuffer(GL_COPY_READ_BUFFER, UBO);

		unsigned typeSize = 0;
		if (TestMask(_data->typeMask, Float)) typeSize = sizeof(GLfloat);
		else if (TestMask(_data->typeMask, Int)) typeSize = sizeof(GLint);
		else if (TestMask(_data->typeMask, Uint)) typeSize = sizeof(GLuint);

		unsigned matX, matY;
		{
			auto tmp = GetMatrixSizes(_data->matSize);
			matX = tmp.first;
			matY = tmp.second;
		}

		void* newData = new char[_data->dataSize];
		if (TestMask(_data->typeMask, Basic))
		{
			if (TestMask(_data->typeMask, Single))
			{
				memcpy(newData, _rawData, typeSize);
			}
			else
			{
				unsigned elementSize = typeSize * 4; // rounds up to vec4 alignmnet

				for (unsigned int i = 0; i < _data->arrayCount; i++)
				{
					memcpy((char*)newData + (elementSize * i), (char*)_rawData + (typeSize * i), typeSize);
				}
			}
		}
		else if (TestMask(_data->typeMask, Vec))
		{
			if (TestMask(_data->typeMask, Single))
			{
				memcpy(newData, _rawData, typeSize * matX);
			}
			else
			{
				unsigned elementSize = typeSize * 4;

				for (unsigned int i = 0; i < _data->arrayCount; i++)
				{
					memcpy((char*)newData + (elementSize * i), (char*)_rawData + (typeSize * matX * i), typeSize * matX);
				}
			}
		}
		else if (TestMask(_data->typeMask, Mat))
		{
			unsigned rows = matX;
			unsigned cols = matY;
			if (_data->typeMask & Single)
			{
				unsigned elementSize = typeSize * 4;

				for (unsigned int i = 0; i < rows; i++)
				{
					memcpy((char*)newData + (elementSize * i), (char*)_rawData + (typeSize * cols * i), typeSize * cols);
				}
			}
			else
			{
				unsigned elementSize = typeSize * 4;
				rows *= _data->arrayCount;

				for (unsigned int i = 0; i < rows; i++)
				{
					memcpy((char*)newData + (elementSize * i), (char*)_rawData + (typeSize * cols * i), typeSize * cols);
				}
			}
		}
		
		glBufferSubData(GL_COPY_READ_BUFFER, _data->dataOffset, _data->dataSize, newData);
	}
	void GlUbo::RemakeUBO(UboData* _change, void* _newData)
	{
		if (_change != nullptr)
		{
			// These two long blocks convert glm types to raw values
			if (TestMask(_change->typeMask, UboDataMask::Vec))
			{
				if (TestMask(_change->matSize, UboMatSize::Size1x2)) { _newData = glm::value_ptr(*(glm::vec2*)_newData); }
				else if (TestMask(_change->matSize, Size1x3)) { _newData = glm::value_ptr(*(glm::vec3*)_newData); }
				else { _newData = glm::value_ptr(*(glm::vec3*)_newData); }
			}
			else if (TestMask(_change->typeMask, UboDataMask::Mat))
			{
				if (TestMask(_change->matSize, UboMatSize::Size1x2))
				{
					if (TestMask(_change->matSize, UboMatSize::Size2x1)) { _newData = glm::value_ptr(*(glm::mat2x2*)_newData); }
					else if (TestMask(_change->matSize, Size3x1)) { _newData = glm::value_ptr(*(glm::mat3x2*)_newData); }
					else { _newData = glm::value_ptr(*(glm::mat4x2*)_newData); }
				}
				else if (TestMask(_change->matSize, Size1x3))
				{
					if (TestMask(_change->matSize, UboMatSize::Size2x1)) { _newData = glm::value_ptr(*(glm::mat2x3*)_newData); }
					else if (TestMask(_change->matSize, Size3x1)) { _newData = glm::value_ptr(*(glm::mat3x3*)_newData); }
					else { _newData = glm::value_ptr(*(glm::mat4x3*)_newData); }
				}
				else
				{
					if (TestMask(_change->matSize, UboMatSize::Size2x1)) { _newData = glm::value_ptr(*(glm::mat2x4*)_newData); }
					else if (TestMask(_change->matSize, Size3x1)) { _newData = glm::value_ptr(*(glm::mat3x4*)_newData); }
					else { _newData = glm::value_ptr(*(glm::mat4x4*)_newData); }
				}
			}

			unsigned preChangeSize = 0;
			if (_change->index > 0) // if it isn't the first in the array
			{
				preChangeSize = (*uboData)[_change->index - 1]->dataOffset + (*uboData)[_change->index - 1]->dataSize;
			}

			unsigned postChangeSize = 0;
			if (_change->index < uboData->size() - 1) // and isn't the last
			{
				postChangeSize = ((*uboData)[uboData->size() - 1]->dataOffset + (*uboData)[uboData->size() - 1]->dataSize) - (*uboData)[_change->index + 1]->dataOffset;
			}

			unsigned typeSize = 0;
			if (TestMask(_change->typeMask, Float)) typeSize = sizeof(GLfloat);
			else if (TestMask(_change->typeMask, Int)) typeSize = sizeof(GLint);
			else if (TestMask(_change->typeMask, Uint)) typeSize = sizeof(GLuint);

			unsigned matX, matY;
			{
				auto tmp = GetMatrixSizes(_change->matSize);
				matX = tmp.first;
				matY = tmp.second;
			}

			glBindBuffer(GL_COPY_READ_BUFFER, UBO);
			void* oldRawData;
			{
				unsigned UBOSize = (*uboData)[uboData->size() - 1]->dataOffset + (*uboData)[uboData->size() - 1]->dataSize;
				oldRawData = new char[UBOSize];
				glGetBufferSubData(GL_COPY_READ_BUFFER, 0, UBOSize, oldRawData);
			}
			void* newRawData;

			unsigned dataSize = 0;
			if (TestMask(_change->typeMask, Basic))
			{
				if (TestMask(_change->typeMask, Single))
				{
					dataSize = typeSize;
					
					newRawData = new char[preChangeSize + postChangeSize + dataSize];

					memcpy((char*)newRawData + _change->dataOffset, _newData, dataSize);
				}
				else
				{
					unsigned elementSize = typeSize * 4; // rounds up to vec4 alignmnet

					dataSize = elementSize * _change->arrayCount;

					newRawData = new char[preChangeSize + postChangeSize + dataSize];
					
					for (unsigned int i = 0; i < _change->arrayCount; i++)
					{
						memcpy((char*)newRawData + _change->dataOffset + (elementSize * i), (char*)_newData + (typeSize * i), typeSize);
					}
				}
			}
			else if (TestMask(_change->typeMask, Vec))
			{
				if (TestMask(_change->typeMask, Single))
				{
					dataSize = typeSize * 2; //aligns to vec2
					if (matX > 2)
						dataSize *= 2; //aligns from vec2 to vec4
					
					newRawData = new char[preChangeSize + postChangeSize + dataSize];

					memcpy((char*)newRawData + _change->dataOffset, _newData, typeSize * matX);
				}
				else
				{
					unsigned elementSize = typeSize * 4;

					dataSize = elementSize * _change->arrayCount;

					newRawData = new char[preChangeSize + postChangeSize + dataSize];

					for (unsigned int i = 0; i < _change->arrayCount; i++)
					{
						memcpy((char*)newRawData + _change->dataOffset + (elementSize * i), (char*)_newData + (typeSize * matX * i), typeSize* matX);
					}
				}
			}
			else if (TestMask(_change->typeMask, Mat))
			{
				unsigned rows = matX;
				unsigned cols = matY;
				if (_change->typeMask & Single)
				{
					unsigned elementSize = typeSize * 4;

					dataSize = elementSize * rows;

					newRawData = new char[preChangeSize + postChangeSize + dataSize];

					for (unsigned int i = 0; i < rows; i++)
					{
						memcpy((char*)newRawData + _change->dataOffset + (elementSize * i), (char*)_newData + (typeSize * cols * i), typeSize * cols);
					}
				}
				else
				{
					unsigned elementSize = typeSize * 4;
					rows *= _change->arrayCount;

					dataSize = elementSize * rows;

					newRawData = new char[preChangeSize + postChangeSize + dataSize];

					for (unsigned int i = 0; i < rows; i++)
					{
						memcpy((char*)newRawData + _change->dataOffset + (elementSize * i), (char*)_newData + (typeSize * cols * i), typeSize * cols);
					}
				}
			}
			else // struct marker
			{
				unsigned offsetFromPad = 16 - (_change->dataOffset % 16);

				if (offsetFromPad != 16)
					dataSize = offsetFromPad;
				else
					dataSize = 0;

				newRawData = new char[preChangeSize + postChangeSize + dataSize];
			}

			memcpy(newRawData, oldRawData, preChangeSize);
			if (postChangeSize > 0)
			{
				memcpy((char*)newRawData + preChangeSize + dataSize, (char*)oldRawData + (*uboData)[_change->index + 1]->dataOffset, postChangeSize);
				unsigned deltaSize = dataSize - _change->dataSize;
				for (unsigned i = _change->index + 1; i < uboData->size(); i++)
				{
					(*uboData)[i]->dataOffset += deltaSize;
				}
			}

			glBufferData(GL_COPY_READ_BUFFER, preChangeSize + postChangeSize + dataSize, newRawData, GL_STATIC_DRAW);
			delete[] oldRawData;
			delete[] newRawData;

			_change->dataSize = dataSize;
		} // TODO: add an else here for when there isn't a specified change. Rebuild the whole buffer from scratch.
	}


	std::pair<unsigned, unsigned> GlUbo::GetMatrixSizes(char _matSize)
	{
		unsigned x = _matSize & 0b0011;
		unsigned y = (_matSize & 0b1100) >> 2;
		return std::pair<unsigned, unsigned>(x + 1, y + 1);
	}

#pragma region GetTypeData() template specializations
#define DefineTypeTemplate(__type__, __type1Enum__, __type2Enum__, __matSizeY__, __matSizeX__) template<> std::pair<char, char> GlUbo::GetTypeData<##__type__##>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | ##__type1Enum__## | ##__type2Enum__##, Size##__matSizeY__##x1 | Size1x##__matSizeX__##); };

	DefineTypeTemplate(float, Basic, Float, 1, 1);
	DefineTypeTemplate(int, Basic, Int, 1, 1);
	DefineTypeTemplate(unsigned, Basic, Uint, 1, 1);
	//template<> std::pair<char, char> GlUbo::GetTypeData<float>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Basic | Float, Size1x1); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<int>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Basic | Int, Size1x1); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<unsigned>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Basic | Uint, Size1x1); };

	DefineTypeTemplate(glm::vec2, Vec, Float, 1, 2);
	DefineTypeTemplate(glm::vec3, Vec, Float, 1, 3);
	DefineTypeTemplate(glm::vec4, Vec, Float, 1, 4);
	DefineTypeTemplate(glm::ivec2, Vec, Int, 1, 2);
	DefineTypeTemplate(glm::ivec3, Vec, Int, 1, 3);
	DefineTypeTemplate(glm::ivec4, Vec, Int, 1, 4);
	DefineTypeTemplate(glm::uvec2, Vec, Uint, 1, 2);
	DefineTypeTemplate(glm::uvec3, Vec, Uint, 1, 3);
	DefineTypeTemplate(glm::uvec4, Vec, Uint, 1, 4);
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::vec2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Float, Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::vec3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Float, Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::vec4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Float, Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::ivec2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Int, Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::ivec3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Int, Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::ivec4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Int, Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::uvec2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Uint, Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::uvec3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Uint, Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::uvec4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Vec | Uint, Size1x4); };

	DefineTypeTemplate(glm::mat2x2, Mat, Float, 2, 2);
	DefineTypeTemplate(glm::mat3x3, Mat, Float, 3, 3);
	DefineTypeTemplate(glm::mat4x4, Mat, Float, 4, 4);
	DefineTypeTemplate(glm::mat2x3, Mat, Float, 2, 3);
	DefineTypeTemplate(glm::mat2x4, Mat, Float, 2, 4);
	DefineTypeTemplate(glm::mat3x2, Mat, Float, 3, 2);
	DefineTypeTemplate(glm::mat3x4, Mat, Float, 3, 4);
	DefineTypeTemplate(glm::mat4x2, Mat, Float, 4, 2);
	DefineTypeTemplate(glm::mat4x3, Mat, Float, 4, 3);
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat2x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size2x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat3x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size3x1 | Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat4x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size4x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat2x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size2x1 | Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat2x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size2x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat3x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size3x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat3x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size3x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat4x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size4x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::mat4x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Float, Size4x1 | Size1x3); };

	DefineTypeTemplate(glm::imat2x2, Mat, Int, 2, 2);
	DefineTypeTemplate(glm::imat3x3, Mat, Int, 3, 3);
	DefineTypeTemplate(glm::imat4x4, Mat, Int, 4, 4);
	DefineTypeTemplate(glm::imat2x3, Mat, Int, 2, 3);
	DefineTypeTemplate(glm::imat2x4, Mat, Int, 2, 4);
	DefineTypeTemplate(glm::imat3x2, Mat, Int, 3, 2);
	DefineTypeTemplate(glm::imat3x4, Mat, Int, 3, 4);
	DefineTypeTemplate(glm::imat4x2, Mat, Int, 4, 2);
	DefineTypeTemplate(glm::imat4x3, Mat, Int, 4, 3);
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat2x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size2x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat3x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size3x1 | Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat4x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size4x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat2x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size2x1 | Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat2x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size2x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat3x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size3x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat3x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size3x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat4x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size4x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::imat4x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Int, Size4x1 | Size1x3); };

	DefineTypeTemplate(glm::umat2x2, Mat, Uint, 2, 2);
	DefineTypeTemplate(glm::umat3x3, Mat, Uint, 3, 3);
	DefineTypeTemplate(glm::umat4x4, Mat, Uint, 4, 4);
	DefineTypeTemplate(glm::umat2x3, Mat, Uint, 2, 3);
	DefineTypeTemplate(glm::umat2x4, Mat, Uint, 2, 4);
	DefineTypeTemplate(glm::umat3x2, Mat, Uint, 3, 2);
	DefineTypeTemplate(glm::umat3x4, Mat, Uint, 3, 4);
	DefineTypeTemplate(glm::umat4x2, Mat, Uint, 4, 2);
	DefineTypeTemplate(glm::umat4x3, Mat, Uint, 4, 3);
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat2x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size2x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat3x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size3x1 | Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat4x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size4x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat2x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size2x1 | Size1x3); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat2x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size2x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat3x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size3x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat3x4>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size3x1 | Size1x4); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat4x2>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size4x1 | Size1x2); };
	//template<> std::pair<char, char> GlUbo::GetTypeData<glm::umat4x3>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | Mat | Uint, Size4x1 | Size1x3); };
#pragma endregion
}
