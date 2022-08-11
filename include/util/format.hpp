#pragma once

#include <string>

namespace Format
{
    /**
     * Format a std::string, using c-style printf.
     * 
     * @param fmt Format string.
     * @param args Variadic arguments used in the format string.
     * 
     * @returns std::string formatted string.
     */
    template <typename... Args>
    std::string String(const std::string &fmt, Args... args)
    {
        auto size = std::snprintf(nullptr, 0, fmt.c_str(), args...);

        std::string result;
        result.resize(size + 1);

        std::snprintf(&result[0], result.size(), fmt.c_str(), args...);

        // Remove null-terminator (not needed for std::string).
        result.pop_back();

        return result;
    }
}
