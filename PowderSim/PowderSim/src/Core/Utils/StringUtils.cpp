#include "StringUtils.h"


namespace powd::utils::string
{
	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return str;
	}

	std::vector<std::string> SplitByDelimiter(std::string str, const std::string& delim)
	{
		std::vector<std::string> out;

		size_t delimPos;
		while ((delimPos = str.find(delim)) != std::string::npos)
		{
			out.push_back(str.substr(0, delimPos));
			str.erase(0, delimPos + 1);
		}
		if (str.length() > 0)
			out.push_back(str);

		return out;
	}

	std::string RemoveTrailing(std::string str, const std::string& delim)
	{
		while (str.substr(0, delim.size()) == delim)
		{
			str.erase(0, delim.size());
		}
		while (str.substr(str.size() - delim.size(), delim.size()) == delim)
		{
			str.erase(str.size() - delim.size(), str.size());
		}

		return str;
	}
}
