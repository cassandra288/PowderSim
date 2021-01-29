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
			case Scalar:
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
	}
	void GlUbo::MarkStructEnd()
	{
		UboData* data = new UboData();

		data->index = uboData->size();
		data->typeMask = (char)StructEnd;

		if (uboData->size() > 0)
		{
			auto prevData = (*uboData)[uboData->size() - 1];
			data->alignedOffset = prevData->alignedOffset + GetTypeSize(prevData); // TODO: fix
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
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);

		unsigned typeSize = GetTypeSize(_data);

		unsigned matX, matY;
		{
			auto tmp = GetMatrixSizes(_data->matSize);
			matX = tmp.first;
			matY = tmp.second;
		}

		unsigned baseAlignment = GetBaseAlignment(_data);

		char* newData = new char[_data->dataSpan];
		if (TestMask(_data->typeMask, Mat))
		{
			unsigned rows = matX;
			unsigned rawElementSize = GetTypeSize(_data) * matY;

			if (TestMask(_data->typeMask, Array))
			{
				for (unsigned mat = 0; mat < _data->arrayCount; mat++)
				{
					void* matPtr = _rawData;
					ConvertGLMTypes(_data, matPtr, mat);

					for (unsigned row = 0; row < rows; row++)
					{
						unsigned trueRow = baseAlignment;
						trueRow *= (row + (rows * mat));
						memcpy(newData + trueRow, (char*)matPtr + (rawElementSize * row), rawElementSize);
					}
				}
			}
			else
			{
				ConvertGLMTypes(_data, _rawData);
				for (unsigned int i = 0; i < rows; i++)
				{
					memcpy(newData + (baseAlignment * i), (char*)_rawData + (rawElementSize * i), rawElementSize);
				}
			}
		}
		else if (TestMask(_data->typeMask, Array))
		{
			if (TestMask(_data->typeMask, Vec))
			{
				unsigned rawSize = GetTypeSize(_data) * matX;
				for (unsigned i = 0; i < _data->arrayCount; i++)
				{
					void* vecPtr = _rawData;
					ConvertGLMTypes(_data, vecPtr, i);
					memcpy(newData + (baseAlignment * i), vecPtr, rawSize);
				}
			}
			else
			{
				unsigned rawSize = GetTypeSize(_data);
				for (unsigned i = 0; i < _data->arrayCount; i++)
				{
					memcpy(newData + (baseAlignment * i), (char*)_rawData + (rawSize * i), rawSize);
				}
			}
		}
		else if (TestMask(_data->typeMask, Single))
		{
			if (TestMask(_data->typeMask, Vec))
			{
				unsigned rawSize = GetTypeSize(_data) * matX;
				ConvertGLMTypes(_data, _rawData);
				memcpy(newData, _rawData, rawSize);
			}
			else
			{
				memcpy(newData, _rawData, _data->dataSpan);
			}
		}
		
		glBufferSubData(GL_UNIFORM_BUFFER, _data->alignedOffset, _data->dataSpan, newData);
	}
	void GlUbo::RemakeUBO(UboData* _change, void* _newData)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);

		unsigned preChangeSize = 0;
		if (_change->index > 0) // if it isn't the first in the array
		{
			preChangeSize = (*uboData)[_change->index - 1]->alignedOffset + (*uboData)[_change->index - 1]->dataSpan;
		}

		unsigned postChangeSize = 0;
		if (_change->index < uboData->size() - 1) // and isn't the last
		{
			postChangeSize = ((*uboData)[uboData->size() - 1]->alignedOffset + (*uboData)[uboData->size() - 1]->dataSpan) - (*uboData)[_change->index + 1]->alignedOffset;
		}

		unsigned matX, matY;
		{
			auto tmp = GetMatrixSizes(_change->matSize);
			matX = tmp.first;
			matY = tmp.second;
		}

		unsigned baseAlignment = GetBaseAlignment(_change);

		void* oldRawData = nullptr;
		{
			unsigned UBOSize = GetUBORawSize();
			if (UBOSize > 0)
			{
				oldRawData = new char[UBOSize];
				glGetBufferSubData(GL_UNIFORM_BUFFER, 0, UBOSize, oldRawData);
			}
		}
		void* newRawData = new char[preChangeSize + postChangeSize + _change->dataSpan];

		if (TestMask(_change->typeMask, Mat))
		{
			unsigned rows = matX;
			unsigned rawElementSize = GetTypeSize(_change) * matY;

			if (TestMask(_change->typeMask, Array))
			{
				for (unsigned mat = 0; mat < _change->arrayCount; mat++)
				{
					void* matPtr = _newData;
					ConvertGLMTypes(_change, matPtr, mat);
					
					for (unsigned row = 0; row < rows; row++)
					{
						unsigned trueRow = baseAlignment;
						trueRow *= (row + (rows * mat));
						memcpy((char*)newRawData + _change->alignedOffset + trueRow, (char*)matPtr + (rawElementSize * row), rawElementSize);
					}
				}
			}
			else
			{
				ConvertGLMTypes(_change, _newData);
				for (unsigned int i = 0; i < rows; i++)
				{
					memcpy((char*)newRawData + _change->alignedOffset + (baseAlignment * i), (char*)_newData + (rawElementSize * i), rawElementSize);
				}
			}
		}
		else if (TestMask(_change->typeMask, Array))
		{
			if (TestMask(_change->typeMask, Vec))
			{
				unsigned rawSize = GetTypeSize(_change) * matX;
				for (unsigned i = 0; i < _change->arrayCount; i++)
				{
					void* vecPtr = _newData;
					ConvertGLMTypes(_change, vecPtr, i);
					memcpy((char*)newRawData + _change->alignedOffset + (baseAlignment * i), vecPtr, rawSize);
				}
			}
			else
			{
				unsigned rawSize = GetTypeSize(_change);
				for (unsigned i = 0; i < _change->arrayCount; i++)
				{
					memcpy((char*)newRawData + _change->alignedOffset + (baseAlignment * i), (char*)_newData + (rawSize * i), rawSize);
				}
			}
		}
		else if (TestMask(_change->typeMask, Single))
		{
			if (TestMask(_change->typeMask, Vec))
			{
				unsigned rawSize = GetTypeSize(_change) * matX;
				ConvertGLMTypes(_change, _newData);
				memcpy((char*)newRawData + _change->alignedOffset, _newData, rawSize);
			}
			else
			{
				memcpy((char*)newRawData + _change->alignedOffset, _newData, _change->dataSpan);
			}
		}

		if (preChangeSize > 0)
			memcpy(newRawData, oldRawData, preChangeSize);
		if (postChangeSize > 0)
		{
			memcpy((char*)newRawData + preChangeSize + _change->dataSpan, (char*)oldRawData + (*uboData)[_change->index + 1]->alignedOffset, postChangeSize);
			for (unsigned i = _change->index + 1; i < uboData->size(); i++)
			{
				unsigned baseAlignment = GetBaseAlignment((*uboData)[i]);
				(*uboData)[i]->alignedOffset = (*uboData)[i - 1]->alignedOffset + (*uboData)[i - 1]->dataSpan;
				(*uboData)[i]->alignedOffset += baseAlignment - ((*uboData)[i]->alignedOffset % baseAlignment);
			}
		}
		glBufferData(GL_UNIFORM_BUFFER, preChangeSize + postChangeSize + _change->dataSpan, newRawData, GL_STATIC_DRAW);
		if (oldRawData != nullptr)
			delete[] oldRawData;
		delete[] newRawData;
		//TODO: Can't delete this for some reason?

		rawUBOSize = (*uboData)[uboData->size() - 1]->alignedOffset + (*uboData)[uboData->size() - 1]->dataSpan;

		/*
		memcpy(newRawData, oldRawData, preChangeSize);
		if (postChangeSize > 0)
		{
			memcpy((char*)newRawData + preChangeSize + dataSize, (char*)oldRawData + (*uboData)[_change->index + 1]->dataOffset, postChangeSize);
			unsigned deltaSize = (dataSize + alignOffset) - (_change->dataSize + _change->alignOffset);
			for (unsigned i = _change->index + 1; i < uboData->size(); i++)
			{
				(*uboData)[i]->dataOffset += deltaSize;
			}
		}

		glBufferData(GL_COPY_READ_BUFFER, preChangeSize + postChangeSize + dataSize, newRawData, GL_STATIC_DRAW);
		delete[] oldRawData;
		delete[] newRawData;

		_change->dataSize = dataSize;
		_change->alignOffset = alignOffset;
		*/

			///

		/*unsigned preChangeSize = 0;
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
		unsigned alignOffset = 0;
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
					dataSize *= 2; //aligns to vec4

				if (_change->dataOffset % dataSize != 0)
					alignOffset = dataSize - (_change->dataOffset % dataSize);
				
				newRawData = new char[preChangeSize + postChangeSize + dataSize + alignOffset];

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
			unsigned deltaSize = (dataSize + alignOffset) - (_change->dataSize + _change->alignOffset);
			for (unsigned i = _change->index + 1; i < uboData->size(); i++)
			{
				(*uboData)[i]->dataOffset += deltaSize;
			}
		}

		glBufferData(GL_COPY_READ_BUFFER, preChangeSize + postChangeSize + dataSize, newRawData, GL_STATIC_DRAW);
		delete[] oldRawData;
		delete[] newRawData;

		_change->dataSize = dataSize;
		_change->alignOffset = alignOffset;*/
	}


	std::pair<unsigned, unsigned> GlUbo::GetMatrixSizes(char _matSize)
	{
		unsigned x = _matSize & 0b0011;
		unsigned y = (_matSize & 0b1100) >> 2;
		return std::pair<unsigned, unsigned>(x + 1, y + 1);
	}

	unsigned GlUbo::GetDataSpan(UboData* _data)
	{
		unsigned typeSize = GetTypeSize(_data);
		unsigned baseAlign = GetBaseAlignment(_data);
		unsigned matX, matY;
		{
			auto matData = GetMatrixSizes(_data->matSize);
			matX = matData.first;
			matY = matData.second;
		}

		if (TestMask(_data->typeMask, Mat))
		{
			unsigned arrSize = matX;
			if (TestMask(_data->typeMask, Array))
			{
				arrSize *= _data->arrayCount;
			}

			return baseAlign * arrSize;
		}
		else if (TestMask(_data->typeMask, Array))
		{
			return baseAlign * _data->arrayCount;
		}
		else
		{
			return baseAlign;
		}
	}
	unsigned GlUbo::GetBaseAlignment(UboData* _data)
	{
		unsigned typeSize = GetTypeSize(_data);

		if (TestMask(_data->typeMask, Array))
			return typeSize * 4; // arrays are always aligned to 4<N>

		else if (TestMask(_data->typeMask, Scalar))
			return typeSize; // scalars are aligned to <N>

		else if (TestMask(_data->typeMask, Vec))
			if (GetMatrixSizes(_data->matSize).first > 2)
				return typeSize * 4; // Vec3 and Vec4 are aligned to 4<N>
			else
				return typeSize * 2; // Vec2 is aligned to 2<N>

		else if (TestMask(_data->typeMask, Mat))
			return typeSize * 4; // Matricies are stored as arrays of vectors and as such are padded to 4<N>

		else
			return 0;
	}
	unsigned GlUbo::GetTypeSize(UboData* _data)
	{
		if (TestMask(_data->typeMask, Float)) return sizeof(GLfloat);
		else if (TestMask(_data->typeMask, Int)) return  sizeof(GLint);
		else if (TestMask(_data->typeMask, Uint)) return sizeof(GLuint);
		else return 0;
	}

	void GlUbo::ConvertGLMTypes(UboData* _data, void* _rawData, unsigned i)
	{
		if (TestMask(_data->typeMask, UboDataMask::Vec))
		{
			if (TestMask(_data->matSize, UboMatSize::Size1x2)) { _rawData = glm::value_ptr(((glm::vec2*)_rawData)[i]); }
			else if (TestMask(_data->matSize, Size1x3)) { _rawData = glm::value_ptr(((glm::vec3*)_rawData)[i]); }
			else { _rawData = glm::value_ptr(((glm::vec3*)_rawData)[i]); }
		}
		else if (TestMask(_data->typeMask, UboDataMask::Mat))
		{
			if (TestMask(_data->matSize, UboMatSize::Size1x2))
			{
				if (TestMask(_data->matSize, UboMatSize::Size2x1)) { _rawData = glm::value_ptr(((glm::mat2x2*)_rawData)[i]); }
				else if (TestMask(_data->matSize, Size3x1)) { _rawData = glm::value_ptr(((glm::mat3x2*)_rawData)[i]); }
				else { _rawData = glm::value_ptr(((glm::mat4x2*)_rawData)[i]); }
			}
			else if (TestMask(_data->matSize, Size1x3))
			{
				if (TestMask(_data->matSize, UboMatSize::Size2x1)) { _rawData = glm::value_ptr(((glm::mat2x3*)_rawData)[i]); }
				else if (TestMask(_data->matSize, Size3x1)) { _rawData = glm::value_ptr(((glm::mat3x3*)_rawData)[i]); }
				else { _rawData = glm::value_ptr(((glm::mat4x3*)_rawData)[i]); }
			}
			else
			{
				if (TestMask(_data->matSize, UboMatSize::Size2x1)) { _rawData = glm::value_ptr(((glm::mat2x4*)_rawData)[i]); }
				else if (TestMask(_data->matSize, Size3x1)) { _rawData = glm::value_ptr(((glm::mat3x4*)_rawData)[i]); }
				else { _rawData = glm::value_ptr(((glm::mat4x4*)_rawData)[i]); }
			}
		}
	}

	void GlUbo::GetData(UboData* _data, void* pData)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		
		glGetBufferSubData(GL_UNIFORM_BUFFER, _data->alignedOffset, GetDataSpan(_data), pData);
	}

#pragma region GetTypeData() template specializations
#define DefineTypeTemplate(__type__, __type1Enum__, __type2Enum__, __matSizeY__, __matSizeX__) template<> std::pair<char, char> GlUbo::GetTypeData<##__type__##>(bool _arr) { return std::pair<char, char>((_arr ? Array : Single) | ##__type1Enum__## | ##__type2Enum__##, Size##__matSizeY__##x1 | Size1x##__matSizeX__##); };

	DefineTypeTemplate(float, Scalar, Float, 1, 1);
	DefineTypeTemplate(int, Scalar, Int, 1, 1);
	DefineTypeTemplate(unsigned, Scalar, Uint, 1, 1);

	DefineTypeTemplate(glm::vec2, Vec, Float, 1, 2);
	DefineTypeTemplate(glm::vec3, Vec, Float, 1, 3);
	DefineTypeTemplate(glm::vec4, Vec, Float, 1, 4);
	DefineTypeTemplate(glm::ivec2, Vec, Int, 1, 2);
	DefineTypeTemplate(glm::ivec3, Vec, Int, 1, 3);
	DefineTypeTemplate(glm::ivec4, Vec, Int, 1, 4);
	DefineTypeTemplate(glm::uvec2, Vec, Uint, 1, 2);
	DefineTypeTemplate(glm::uvec3, Vec, Uint, 1, 3);
	DefineTypeTemplate(glm::uvec4, Vec, Uint, 1, 4);

	DefineTypeTemplate(glm::mat2x2, Mat, Float, 2, 2);
	DefineTypeTemplate(glm::mat3x3, Mat, Float, 3, 3);
	DefineTypeTemplate(glm::mat4x4, Mat, Float, 4, 4);
	DefineTypeTemplate(glm::mat2x3, Mat, Float, 2, 3);
	DefineTypeTemplate(glm::mat2x4, Mat, Float, 2, 4);
	DefineTypeTemplate(glm::mat3x2, Mat, Float, 3, 2);
	DefineTypeTemplate(glm::mat3x4, Mat, Float, 3, 4);
	DefineTypeTemplate(glm::mat4x2, Mat, Float, 4, 2);
	DefineTypeTemplate(glm::mat4x3, Mat, Float, 4, 3);

	DefineTypeTemplate(glm::imat2x2, Mat, Int, 2, 2);
	DefineTypeTemplate(glm::imat3x3, Mat, Int, 3, 3);
	DefineTypeTemplate(glm::imat4x4, Mat, Int, 4, 4);
	DefineTypeTemplate(glm::imat2x3, Mat, Int, 2, 3);
	DefineTypeTemplate(glm::imat2x4, Mat, Int, 2, 4);
	DefineTypeTemplate(glm::imat3x2, Mat, Int, 3, 2);
	DefineTypeTemplate(glm::imat3x4, Mat, Int, 3, 4);
	DefineTypeTemplate(glm::imat4x2, Mat, Int, 4, 2);
	DefineTypeTemplate(glm::imat4x3, Mat, Int, 4, 3);

	DefineTypeTemplate(glm::umat2x2, Mat, Uint, 2, 2);
	DefineTypeTemplate(glm::umat3x3, Mat, Uint, 3, 3);
	DefineTypeTemplate(glm::umat4x4, Mat, Uint, 4, 4);
	DefineTypeTemplate(glm::umat2x3, Mat, Uint, 2, 3);
	DefineTypeTemplate(glm::umat2x4, Mat, Uint, 2, 4);
	DefineTypeTemplate(glm::umat3x2, Mat, Uint, 3, 2);
	DefineTypeTemplate(glm::umat3x4, Mat, Uint, 3, 4);
	DefineTypeTemplate(glm::umat4x2, Mat, Uint, 4, 2);
	DefineTypeTemplate(glm::umat4x3, Mat, Uint, 4, 3);
#pragma endregion
}
