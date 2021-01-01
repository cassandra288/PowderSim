#pragma once

#include <unordered_map>

#include <glm/glm.hpp>


namespace powd::rendering
{
	class GlUbo
	{
		struct UboData
		{
			void* data;
			unsigned dataCount;
			unsigned arraySize;
			unsigned padSize;
			char padMask; //0x - 00 (0 = float, 1 = int, 2 = uint) - 00 (0 = basic, 1 = vec, 2 = mat) - 0 (0 = single, 1 = array)
			unsigned startByte;
		};

		enum PadMasks
		{
			Single = 0b00000,
			Array = 0b00001,

			Basic = 0b00010,
			Vec = 0b00100,
			Mat = 0b00110,

			Float = 0b01000,
			Int = 0b10000,
			Uint = 0b11000
		};

		bool CompareMask(char mask1, PadMasks mask2)
		{
			if (mask2 == Single || mask2 == Array)
				return (mask1 & 0b00001) == (mask2 & 0b00001);
			else if (mask2 == Basic || mask2 == Vec || mask2 == Mat)
				return (mask1 & 0b00110) == (mask2 & 0b00110);
			else
				return (mask1 & 0b11000) == (mask2 & 0b11000);
			return false;
		}

		bool uboLoaded;
		std::unordered_map<unsigned, UboData> uboDataMap;
		unsigned endID; // this is the largest index in the UBO data map

		unsigned ubo;

	public:
		GlUbo();
		~GlUbo();

		void ClearUbo();
		void BindUbo();

		void ReloadUBO();

#pragma region UboDataFunctions
		//scalars
		void SetUniform(unsigned pos, float* val);
		void SetUniform(unsigned pos, int* val);
		void SetUniform(unsigned pos, unsigned* val);
		void SetUniform(unsigned pos, float* val, unsigned count);
		void SetUniform(unsigned pos, int* val, unsigned count);
		void SetUniform(unsigned pos, unsigned* val, unsigned count);

		//vectors
		void SetUniform(unsigned pos, glm::vec2* val);
		void SetUniform(unsigned pos, glm::vec3* val);
		void SetUniform(unsigned pos, glm::vec4* val);
		void SetUniform(unsigned pos, glm::ivec2* val);
		void SetUniform(unsigned pos, glm::ivec3* val);
		void SetUniform(unsigned pos, glm::ivec4* val);
		void SetUniform(unsigned pos, glm::uvec2* val);
		void SetUniform(unsigned pos, glm::uvec3* val);
		void SetUniform(unsigned pos, glm::uvec4* val);
		void SetUniform(unsigned pos, glm::vec2* val, unsigned count);
		void SetUniform(unsigned pos, glm::vec3* val, unsigned count);
		void SetUniform(unsigned pos, glm::vec4* val, unsigned count);
		void SetUniform(unsigned pos, glm::ivec2* val, unsigned count);
		void SetUniform(unsigned pos, glm::ivec3* val, unsigned count);
		void SetUniform(unsigned pos, glm::ivec4* val, unsigned count);
		void SetUniform(unsigned pos, glm::uvec2* val, unsigned count);
		void SetUniform(unsigned pos, glm::uvec3* val, unsigned count);
		void SetUniform(unsigned pos, glm::uvec4* val, unsigned count);

		//matrices
		void SetUniform(unsigned pos, glm::mat2x2* val);
		void SetUniform(unsigned pos, glm::mat3x3* val);
		void SetUniform(unsigned pos, glm::mat4x4* val);
		void SetUniform(unsigned pos, glm::mat2x3* val);
		void SetUniform(unsigned pos, glm::mat3x2* val);
		void SetUniform(unsigned pos, glm::mat2x4* val);
		void SetUniform(unsigned pos, glm::mat4x2* val);
		void SetUniform(unsigned pos, glm::mat3x4* val);
		void SetUniform(unsigned pos, glm::mat4x3* val);
		void SetUniform(unsigned pos, glm::mat2x2* val, unsigned count);
		void SetUniform(unsigned pos, glm::mat3x3* val, unsigned count);
		void SetUniform(unsigned pos, glm::mat4x4* val, unsigned count);
		void SetUniform(unsigned pos, glm::mat2x3* val, unsigned count);
		void SetUniform(unsigned pos, glm::mat3x2* val, unsigned count);
		void SetUniform(unsigned pos, glm::mat2x4* val, unsigned count);
		void SetUniform(unsigned pos, glm::mat4x2* val, unsigned count);
		void SetUniform(unsigned pos, glm::mat3x4* val, unsigned count);
		void SetUniform(unsigned pos, glm::mat4x3* val, unsigned count);


		void UpdateUniform(unsigned pos, void* data);
#pragma endregion
	};
}
