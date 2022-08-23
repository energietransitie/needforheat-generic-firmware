#pragma once

#include <string>
#include <vector>

namespace Strings
{
	/**
	 * Split a string according to a specified delimeter.
	 *
	 * @param str The string to split.
	 * @param delimiter The delimeter where to split the string.
	 * 
	 * @returns A vector of strings, split by the delimiter.
	 */
	std::vector<std::string> Split(const std::string &str, const char &delimiter);
} // namespace Strings
