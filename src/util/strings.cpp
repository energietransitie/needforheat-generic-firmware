#include <util/strings.hpp>

namespace Strings
{
	std::vector<std::string> Split(const std::string &str, const char &delimiter)
	{
		std::vector<std::string> subStrings;
		std::string subString;

		for (const auto &c : str)
		{
			if (c == delimiter)
			{
				// Delimeter found. Add substring to vector
				// and clear current substring.
				subStrings.push_back(subString);
				subString.clear();
			}
			else
			{
				// Add the character to the current substring.
				subString += c;
			}
		}

		if (!subString.empty())
		{
			subStrings.push_back(subString);
		}

		return subStrings;
	}
} // namespace Strings
