#pragma once

#include <string>

#include <glm/glm.hpp>


namespace powd::sand
{
	typedef unsigned Powder;

	void SandEngineSetup();
	void SandEngineShutdown();

	bool CreateNewPowder(std::string id, glm::uvec2 pos, Powder& idOut);
	void RemovePowder(Powder powd);

	glm::uvec2 GetPowderPos(Powder powder);
	bool SetPowderPos(glm::uvec2 newPos, Powder powder);
	bool TranslatePowderPos(glm::uvec2 translation, Powder powder);
}
