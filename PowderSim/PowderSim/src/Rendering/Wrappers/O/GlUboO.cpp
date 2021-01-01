#include "GlUboO.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <CppLog/Logger.h>

#include "src/Core/Exceptions/GenericExceptions.h"

USING_LOGGER


namespace powd::rendering
{
	GlUbo::GlUbo()
	{
		uboLoaded = false;
		uboDataMap = {};
		endID = 0;

		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, 0, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);
	}
	GlUbo::~GlUbo()
	{
		glDeleteBuffers(1, &ubo);
		ClearUbo();
	}

	void GlUbo::ClearUbo()
	{
		std::list<unsigned> ids;
		for (auto it = uboDataMap.begin(); it != uboDataMap.end(); it++)
			ids.push_back(it->first);

		for (unsigned id : ids)
		{
			delete& uboDataMap[id];
		}

		uboDataMap.clear();
		endID = 0;
		uboLoaded = false;
	}
	void GlUbo::BindUbo()
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);
	}

	void GlUbo::ReloadUBO()
	{
		unsigned size = 0;
		for (unsigned i = 0; i <= endID; i++)
		{
			if (uboDataMap.find(i) == uboDataMap.end()) // error checking to see if the data has gaps in it (which will cause problems)
				throw exceptions::GenericException("Attempting to reload UBO with gaps in the data.", __FILE__, __LINE__);

			size += (uboDataMap[i].dataCount * uboDataMap[i].arraySize);
			if (CompareMask(uboDataMap[i].padMask, PadMasks::Mat))
			{
				size += ((uboDataMap[i].padSize * (4 - uboDataMap[i].padSize)) * uboDataMap[i].arraySize);
			}
			else //first we could up the total size (in sets of 4 bytes which is a float/int/uint)
			{
				size += (uboDataMap[i].padSize * uboDataMap[i].arraySize);
			}
			if (size % 4 != 0)
				size += size % 4;
		}

		void* finishedData = new float[size]; // here we store a pointer to the whole array and also use a pointer to index through the array.
		char* dataPointer = (char*)finishedData; // we need to do it like this because of the array being of type void (which allows it to be typeless)
		for (unsigned i = 0; i <= endID; i++)
		{
			uboDataMap[i].startByte = dataPointer - (char*)finishedData;

			if (CompareMask(uboDataMap[i].padMask, PadMasks::Float))
			{
				float* data = (float*)uboDataMap[i].data;
				if (CompareMask(uboDataMap[i].padMask, PadMasks::Single))
				{
					if (CompareMask(uboDataMap[i].padMask, PadMasks::Basic))
					{
						// single basic (doesn't need padding)
						*(float*)dataPointer = data[0];
						dataPointer += sizeof(float);
					}
					else if (CompareMask(uboDataMap[i].padMask, PadMasks::Vec))
					{
						// single vector
						memcpy(dataPointer, &data[0], uboDataMap[i].dataCount * sizeof(float));
						dataPointer += sizeof(float) * (uboDataMap[i].padSize + uboDataMap[i].dataCount); // skip padding data
					}
					else
					{
						// single matrix
						for (unsigned row = 0; row < uboDataMap[i].dataCount / (4 - uboDataMap[i].padSize); row++)
						{
							memcpy(dataPointer, &data[row * (4 - uboDataMap[i].padSize)], (4 - uboDataMap[i].padSize) * sizeof(float));
							dataPointer += sizeof(float) * 4;
						}
					}
				}
				else
				{
					if (CompareMask(uboDataMap[i].padMask, PadMasks::Basic))
					{
						// array basic
						for (unsigned j = 0; j < uboDataMap[i].arraySize; j++)
						{
							*(float*)dataPointer = data[j];
							dataPointer += sizeof(float) * (uboDataMap[i].padSize + 1); // skip padding data
						}

					}
					else if (CompareMask(uboDataMap[i].padMask, PadMasks::Vec))
					{
						// array vector
						for (unsigned j = 0; j < uboDataMap[i].arraySize * uboDataMap[i].dataCount; j += uboDataMap[i].dataCount)
						{
							memcpy(dataPointer, &data[j], uboDataMap[i].dataCount * sizeof(float));
							dataPointer += sizeof(float) * 4; // array vectors are expected to be vec4 in size
						}
					}
					else
					{
						// array matrix
						for (unsigned j = 0; j < uboDataMap[i].arraySize; j++)
						{
							for (unsigned row = 0; row < uboDataMap[i].dataCount / (4 - uboDataMap[i].padSize); row++)
							{
								memcpy(dataPointer, &data[(j * uboDataMap[i].dataCount) + (row * (4 - uboDataMap[i].padSize))], (4 - uboDataMap[i].padSize) * sizeof(float));
								dataPointer += sizeof(float) * 4;
							}
						}
					}
				}
			}
			else if (CompareMask(uboDataMap[i].padMask, PadMasks::Int))
			{
				int* data = (int*)uboDataMap[i].data;
				if (CompareMask(uboDataMap[i].padMask, PadMasks::Single))
				{
					if (CompareMask(uboDataMap[i].padMask, PadMasks::Basic))
					{
						// single basic (doesn't need padding)
						*(int*)dataPointer = data[0];
						dataPointer += sizeof(int);
					}
					else if (CompareMask(uboDataMap[i].padMask, PadMasks::Vec))
					{
						// single vector
						memcpy(dataPointer, &data[0], uboDataMap[i].dataCount * sizeof(int));
						dataPointer += sizeof(int) * (uboDataMap[i].padSize + uboDataMap[i].dataCount); // skip padding data
					}
					else
					{
						// single matrix
						for (unsigned row = 0; row < uboDataMap[i].dataCount / (4 - uboDataMap[i].padSize); row++)
						{
							memcpy(dataPointer, &data[row * (4 - uboDataMap[i].padSize)], (4 - uboDataMap[i].padSize) * sizeof(int));
							dataPointer += sizeof(int) * 4;
						}
					}
				}
				else
				{
					if (CompareMask(uboDataMap[i].padMask, PadMasks::Basic))
					{
						// array basic
						for (unsigned j = 0; j < uboDataMap[i].arraySize; j++)
						{
							*(int*)dataPointer = data[j];
							dataPointer += sizeof(int) * (uboDataMap[i].padSize + 1); // skip padding data
						}

					}
					else if (CompareMask(uboDataMap[i].padMask, PadMasks::Vec))
					{
						// array vector
						for (unsigned j = 0; j < uboDataMap[i].arraySize * uboDataMap[i].dataCount; j += uboDataMap[i].dataCount)
						{
							memcpy(dataPointer, &data[j], uboDataMap[i].dataCount * sizeof(int));
							dataPointer += sizeof(int) * 4; // array vectors are expected to be vec4 in size
						}
					}
					else
					{
						// array matrix
						for (unsigned j = 0; j < uboDataMap[i].arraySize; j++)
						{
							for (unsigned row = 0; row < uboDataMap[i].dataCount / (4 - uboDataMap[i].padSize); row++)
							{
								memcpy(dataPointer, &data[(j * uboDataMap[i].dataCount) + (row * (4 - uboDataMap[i].padSize))], (4 - uboDataMap[i].padSize) * sizeof(int));
								dataPointer += sizeof(int) * 4;
							}
						}
					}
				}
			}
			else
			{
				unsigned* data = (unsigned*)uboDataMap[i].data;
				if (CompareMask(uboDataMap[i].padMask, PadMasks::Single))
				{
					if (CompareMask(uboDataMap[i].padMask, PadMasks::Basic))
					{
						// single basic (doesn't need padding)
						*(unsigned*)dataPointer = data[0];
						dataPointer += sizeof(unsigned);
					}
					else if (CompareMask(uboDataMap[i].padMask, PadMasks::Vec))
					{
						// single vector
						memcpy(dataPointer, &data[0], uboDataMap[i].dataCount * sizeof(unsigned));
						dataPointer += sizeof(unsigned) * (uboDataMap[i].padSize + uboDataMap[i].dataCount); // skip padding data
					}
					else
					{
						// single matrix
						for (unsigned row = 0; row < uboDataMap[i].dataCount / (4 - uboDataMap[i].padSize); row++)
						{
							memcpy(dataPointer, &data[row * (4 - uboDataMap[i].padSize)], (4 - uboDataMap[i].padSize) * sizeof(unsigned));
							dataPointer += sizeof(unsigned) * 4;
						}
					}
				}
				else
				{
					if (CompareMask(uboDataMap[i].padMask, PadMasks::Basic))
					{
						// array basic
						for (unsigned j = 0; j < uboDataMap[i].arraySize; j++)
						{
							*(unsigned*)dataPointer = data[j];
							dataPointer += sizeof(unsigned) * (uboDataMap[i].padSize + 1); // skip padding data
						}

					}
					else if (CompareMask(uboDataMap[i].padMask, PadMasks::Vec))
					{
						// array vector
						for (unsigned j = 0; j < uboDataMap[i].arraySize * uboDataMap[i].dataCount; j += uboDataMap[i].dataCount)
						{
							memcpy(dataPointer, &data[j], uboDataMap[i].dataCount * sizeof(unsigned));
							dataPointer += sizeof(unsigned) * 4; // array vectors are expected to be vec4 in size
						}
					}
					else
					{
						// array matrix
						for (unsigned j = 0; j < uboDataMap[i].arraySize; j++)
						{
							for (unsigned row = 0; row < uboDataMap[i].dataCount / (4 - uboDataMap[i].padSize); row++)
							{
								memcpy(dataPointer, &data[(j * uboDataMap[i].dataCount) + (row * (4 - uboDataMap[i].padSize))], (4 - uboDataMap[i].padSize) * sizeof(unsigned));
								dataPointer += sizeof(unsigned) * 4;
							}
						}
					}
				}
			}

			if ((dataPointer - (char*)finishedData) / sizeof(float) % 4 != 0)
			{
				dataPointer += ((dataPointer - (char*)finishedData) % 4);
			}
		}

		// finally bind the data to the UBO
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, size * sizeof(float), finishedData, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		uboLoaded = true;
	}

	//I sincerely apologise
	//do NOT open this you will lose your place in the code
#pragma region UboDataFunctions
	//scalars
	void GlUbo::SetUniform(unsigned pos, float* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = val;
		uboDataMap[pos].dataCount = 1;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Basic | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, int* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = val;
		uboDataMap[pos].dataCount = 1;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Int | PadMasks::Basic | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, unsigned* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = val;
		uboDataMap[pos].dataCount = 1;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Uint | PadMasks::Basic | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, float* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = &val;
		uboDataMap[pos].dataCount = 1;
		uboDataMap[pos].padSize = 3;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Basic | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, int* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = &val;
		uboDataMap[pos].dataCount = 1;
		uboDataMap[pos].padSize = 3;
		uboDataMap[pos].padMask = PadMasks::Int | PadMasks::Basic | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, unsigned* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = &val;
		uboDataMap[pos].dataCount = 1;
		uboDataMap[pos].padSize = 3;
		uboDataMap[pos].padMask = PadMasks::Uint | PadMasks::Basic | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}

	//vectors
	void GlUbo::SetUniform(unsigned pos, glm::vec2* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 2;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::vec3* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 3;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::vec4* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 4;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::ivec2* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 2;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Int | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::ivec3* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 3;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Int | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::ivec4* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 4;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Int | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::uvec2* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 2;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Uint | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::uvec3* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 3;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Uint | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::uvec4* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 4;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Uint | PadMasks::Vec | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::vec2* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 2;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::vec3* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 3;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::vec4* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 4;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::ivec2* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 2;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Int | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::ivec3* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 3;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Int | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::ivec4* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 4;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Int | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::uvec2* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 2;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Uint | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::uvec3* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 3;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Uint | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::uvec4* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 4;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Uint | PadMasks::Vec | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}

	//matrices
	void GlUbo::SetUniform(unsigned pos, glm::mat2x2* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 4;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat3x3* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 9;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat4x4* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 16;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat2x3* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 6;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat3x2* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 6;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat2x4* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 8;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat4x2* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 8;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat3x4* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 12;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat4x3* val)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(*val);
		uboDataMap[pos].dataCount = 12;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Single;
		uboDataMap[pos].arraySize = 1;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat2x2* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 4;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat3x3* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 9;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat4x4* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 16;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat2x3* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 6;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat3x2* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 6;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat2x4* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 8;
		uboDataMap[pos].padSize = 2;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat4x2* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 8;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat3x4* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 12;
		uboDataMap[pos].padSize = 1;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}
	void GlUbo::SetUniform(unsigned pos, glm::mat4x3* val, unsigned count)
	{
		if (uboDataMap.find(pos) == uboDataMap.end())
		{
			uboDataMap[pos] = *new UboData();
		}
		uboDataMap[pos].data = glm::value_ptr(val[0]);
		uboDataMap[pos].dataCount = 12;
		uboDataMap[pos].padSize = 0;
		uboDataMap[pos].padMask = PadMasks::Float | PadMasks::Mat | PadMasks::Array;
		uboDataMap[pos].arraySize = count;
		uboDataMap[pos].startByte = 0;
		uboLoaded = false;

		if (endID < pos)
			endID = pos;
	}


	void GlUbo::UpdateUniform(unsigned pos, void* data)
	{
		if (uboLoaded)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, uboDataMap[pos].startByte, uboDataMap[pos].dataCount * sizeof(float), data);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			Logger::Log("Attempting to update uniform before UBO is loaded", Logger::WARNING);
		}
	}

#pragma endregion
}
