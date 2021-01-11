#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <stack>
#include <sstream>
#include <iomanip>


namespace powd::profiling
{
	namespace intern
	{
		struct SectionTimeNode
		{
		public:
			std::string name = "";
			SectionTimeNode* parent = nullptr;
			std::vector<SectionTimeNode*> children = {};
			std::chrono::steady_clock::duration time;
			std::chrono::steady_clock::duration pausedTime = std::chrono::seconds(0);

			~SectionTimeNode()
			{
				for (SectionTimeNode* child : children)
				{
					delete child;
				}
			}
		};

		void StartFrame(); // special behaviour for the root node
	}


	void StartSectionProfile(std::string _name);

	void StopSectionProfile();

	void PauseSection();
	void UnpauseSection();

	std::string GetProfileDataStr();
}
