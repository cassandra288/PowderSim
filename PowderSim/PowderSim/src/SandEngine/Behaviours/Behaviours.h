#pragma once

#include <string>


namespace powd::sand
{
	void LoadBehaviours();

	void UnloadBehaviours();
	
	bool RunBehaviour(unsigned powder, std::string behaviour);
	void RunBuiltinBehaviour(std::string behaviour);
}
