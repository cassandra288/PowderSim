#pragma once

#include <vector>

#include "src/Core/Exceptions/GenericExceptions.h"


namespace powd::rendering
{
	class GlUbo;

	struct UboData
	{
	private:
		char typeMask = 0; //0b - 00 (01 = structStart, 10 = structEnd) - 00 (01 = float, 10 = int, 11 = uint) - 00 (01 = basic, 10 = vec, 11 = mat) - 00 (01 = single, 10 = array)
		char matSize = 0;
		unsigned arrayCount = 0;
		unsigned index = 0;

		unsigned dataSpan;
		unsigned alignedOffset;

		friend GlUbo;
	};

	enum UboDataMask
	{
		Single = 0b00000001,
		Array = 0b00000010,

		Scalar = 0b00000100,
		Vec = 0b00001000,
		Mat = 0b00001100,

		Float = 0b00010000,
		Int = 0b00100000,
		Uint = 0b00110000,

		StructStart = 0b01000000,
		StructEnd = 0b10000000
	};

	enum UboMatSize
	{
		Size1x1 = 0b0000,
		Size1x2 = 0b0001,
		Size1x3 = 0b0010,
		Size1x4 = 0b0011,

		Size2x1 = 0b0100,
		Size3x1 = 0b1000,
		Size4x1 = 0b1100
	};
	
	class GlUbo
	{
	private:
		unsigned UBO = 0;
		unsigned rawUBOSize;
		std::vector<UboData*>* uboData = {};
		unsigned referenceCounter = 0;

		template<typename T>
		static std::pair<char, char> GetTypeData(bool _arr); //returns: typeMask, matSize

		static std::pair<unsigned, unsigned> GetMatrixSizes(char _matSize); //returns: x, y
		unsigned GetDataSpan(UboData* _data);
		unsigned GetBaseAlignment(UboData* _data);
		unsigned GetTypeSize(UboData* _data);
		void ConvertGLMTypes(UboData* _data, void* _rawData, unsigned i = 0);
		void GetData(UboData* _data, void* pData);

		void RemakeUBO(UboData* _change = nullptr, void* _newData = nullptr); // TODO: Fix for structs.
		void RewriteData(UboData* _data, void* _rawData);

	public:
		GlUbo();
		~GlUbo();
		GlUbo(GlUbo& _o);
		GlUbo(GlUbo&& _o) noexcept;
		GlUbo& operator =(GlUbo& _o);
		GlUbo& operator =(GlUbo&& _o) noexcept;

		void ClearUBO();
		void SetUBOSize(unsigned _size);
		void MarkStructEnd();

		void Bind(unsigned _i);

		unsigned GetUBORawSize()
		{
			return rawUBOSize;
		}
		unsigned GetUBODataCount()
		{
			return uboData->size();
		}


	public:
		template<typename T>
		void AddData(T* _data, unsigned _arrayCount = 1)
		{
			char typeMask;
			char matSize;
			{
				std::pair<char, char> typeData = GetTypeData<T>(_arrayCount > 1);
				typeMask = typeData.first;
				matSize = typeData.second;
			}

			UboData* data = new UboData();

			data->typeMask = typeMask;
			data->matSize = matSize;
			data->arrayCount = _arrayCount;
			data->index = uboData->size();

			data->dataSpan = GetDataSpan(data);
			data->alignedOffset = 0;
			if (data->index > 0)
			{
				unsigned baseAlignment = GetBaseAlignment(data);
				data->alignedOffset = (*uboData)[data->index - 1]->alignedOffset + (*uboData)[data->index - 1]->dataSpan;
				data->alignedOffset += baseAlignment - (data->alignedOffset % baseAlignment);
			}

			uboData->push_back(data);

			RemakeUBO(data, _data);
		}

		template<typename T>
		void ModifyData(unsigned _iData, T* _newData, unsigned _newArrayCount = 1)
		{
			if (_iData >= uboData->size())
				throw exceptions::GenericException("UBO doesn't have this many fields.", __FILE__, __LINE__, "Out Of Bounds");

			UboData* data = (*uboData)[_iData];

			char typeMask;
			char matSize;
			{
				std::pair<char, char> typeData = GetTypeData<T>(_newArrayCount > 1);
				typeMask = typeData.first;
				matSize = typeData.second;
			}

			if (data->typeMask == typeMask && data->matSize == matSize && data->arrayCount == _newArrayCount)
			{
				RewriteData(data, _newData);
			}
			else
			{
				data->typeMask = typeMask;
				data->matSize = matSize;
				data->arrayCount = _newArrayCount;

				data->dataSpan = GetDataSpan(data);
				data->alignedOffset = 0;
				if (data->index > 0)
				{
					unsigned baseAlignment = GetBaseAlignment(data);
					data->alignedOffset = (*uboData)[data->index - 1]->alignedOffset + (*uboData)[data->index - 1]->dataSpan;
					data->alignedOffset += baseAlignment - (data->alignedOffset % baseAlignment);
				}
				RemakeUBO(data, _newData);
			}
		}
	};
}
