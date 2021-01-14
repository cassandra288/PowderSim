#include "CPUProfiler.h"


namespace powd::profiling
{
	namespace intern
	{
		std::chrono::steady_clock clck;

		SectionTimeNode rootNode = {
			"Root",
			nullptr,
			{},
			{}
		};

		struct CachedTimeNode
		{
			std::string name;
			std::string time;
			std::string percents;
			std::vector<CachedTimeNode*> children;

			~CachedTimeNode()
			{
				for (CachedTimeNode* child : children)
				{
					delete child;
				}
			}
		};

		std::stack<std::chrono::steady_clock::time_point> timeStack; // stores time points for when a section starts
		SectionTimeNode* currentNode = nullptr; // stores the currently-profiled node
		uint32_t biggestNameSize = 11; // start at 11 for "Unspecified"

		CachedTimeNode* cachedNodeRoot = nullptr;
		unsigned cachedBiggestName;

		bool paused = false;
		std::chrono::steady_clock::time_point pauseStart;

		void StartFrame() // special behaviour for the root node
		{
			while (timeStack.size() > 0)
			{
				timeStack.pop();
			}

			timeStack.push(clck.now());
			currentNode = &rootNode;
			for (SectionTimeNode* rootChild : currentNode->children)
			{
				delete rootChild;
			}
			currentNode->children.resize(0);
			currentNode->pausedTime = std::chrono::seconds(0);
		}

		void IterateNode(CachedTimeNode* _node, std::stringstream& _os)
		{
			static uint32_t indent = 0;
			static bool spacerIndentDone = false;
			spacerIndentDone = false;

			std::string indentTree;
			{
				for (uint32_t i = 0; i < indent; i++)
				{
					indentTree += "|";
				}
				indentTree += "-";
			}

			std::string name;
			{
				std::stringstream nameStream;
				nameStream << std::setw(((uint64_t)biggestNameSize + 20ULL) - indent) << std::left;
				nameStream << _node->name;
				name = nameStream.str();
			}
			
			_os << indentTree << name << _node->time << _node->percents << std::endl;

			indent++;
			for (auto childNode : _node->children)
			{
				IterateNode(childNode, _os);
			}

			if (!spacerIndentDone)
			{
				spacerIndentDone = true;
				indentTree.resize(indentTree.size() - 1);
				_os << indentTree << std::endl;
			}
			indent--;
		}

		void CacheNode(SectionTimeNode* _node, CachedTimeNode* _cache, bool unspec = false)
		{
			std::string time;
			{
				std::stringstream timeStream;
				timeStream << std::setw(10) << std::setprecision(1) << std::fixed;
				float timeNum = _node->time.count() / 1000000.f;
				if (timeNum < 1)
				{
					timeStream << timeNum * 1000.f;
					time = timeStream.str() + "us  ";
				}
				else
				{
					timeStream << timeNum;
					time = timeStream.str() + "ms  ";
				}
			}

			std::string percentString;
			if (_node->parent == nullptr)
			{ // special handling for the root node
				percentString = "100.0%  100.0%";
			}
			else
			{
				float parentContribution = ((float)_node->time.count() / (float)_node->parent->time.count()) * 100;
				float totalContribution = ((float)_node->time.count() / (float)rootNode.time.count()) * 100;

				std::stringstream percentStream;
				percentStream << std::setw(5) << std::setprecision(1) << std::fixed;

				percentStream << parentContribution;
				percentString = percentStream.str() + "%  ";

				percentStream.str("");

				percentStream << totalContribution;
				percentString += percentStream.str() + "%";
			}

			_cache->name = _node->name;
			_cache->time = time;
			_cache->percents = percentString;

			auto timeLeft = _node->time;
			for (SectionTimeNode* child : _node->children)
			{
				auto newNode = new intern::CachedTimeNode();
				_cache->children.push_back(newNode);

				intern::CacheNode(child, newNode);

				timeLeft -= child->time;
			}

			if (timeLeft.count() > 0 && !unspec)
			{
				SectionTimeNode unspecNode;
				unspecNode.name = "Unspecified";
				unspecNode.parent = _node;
				unspecNode.children = {};
				unspecNode.time = timeLeft;

				auto newNode = new intern::CachedTimeNode();
				_cache->children.push_back(newNode);

				intern::CacheNode(&unspecNode, newNode, true);
			}
		}
	}


	void StartSectionProfile(std::string _name)
	{
		intern::timeStack.push(intern::clck.now());
		intern::SectionTimeNode* newNode = new intern::SectionTimeNode;
		newNode->name = _name;
		newNode->parent = intern::currentNode;
		intern::currentNode->children.push_back(newNode);
		intern::currentNode = newNode;

		if (_name.size() > intern::biggestNameSize)
		{
			intern::biggestNameSize = _name.size();
		}
	}

	void StopSectionProfile()
	{
		if (intern::paused) UnpauseSection();

		std::chrono::steady_clock::time_point now = intern::clck.now();
		intern::currentNode->time = now - intern::timeStack.top();
		intern::currentNode->time -= intern::currentNode->pausedTime;
		intern::timeStack.pop();

		intern::currentNode = intern::currentNode->parent;

		intern::cachedBiggestName = intern::biggestNameSize;
		if (intern::currentNode == nullptr)
		{
			delete intern::cachedNodeRoot;
			intern::cachedNodeRoot = new intern::CachedTimeNode();

			intern::CacheNode(&intern::rootNode, intern::cachedNodeRoot);
		}
	}

	void PauseSection()
	{
		intern::paused = true;
		intern::pauseStart = intern::clck.now();
	}

	void UnpauseSection()
	{
		std::chrono::steady_clock::duration pausedTime = intern::clck.now() - intern::pauseStart;
		intern::paused = false;
		intern::currentNode->pausedTime += pausedTime;

		intern::SectionTimeNode* _node = intern::currentNode->parent;

		while (_node != nullptr)
		{
			_node->pausedTime += pausedTime;
			_node = _node->parent;
		}
	}

	std::string GetProfileDataStr()
	{
		std::stringstream strm;

		if (intern::cachedNodeRoot != nullptr)
			intern::IterateNode(intern::cachedNodeRoot, strm);

		return strm.str();
	}
}
