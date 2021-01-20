#pragma once

#include <string>
#include <vector>


namespace powd::utils::string
{
	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

	std::vector<std::string> SplitByDelimiter(std::string str, const std::string& delim);

	std::string RemoveTrailing(std::string str, const std::string& delim = " ");
}
