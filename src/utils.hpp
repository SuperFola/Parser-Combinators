#ifndef SRC_UTILS_HPP
#define SRC_UTILS_HPP

#include <cmath>
#include <string>

namespace Utils
{
    inline bool isDouble(const std::string& s, double* output = nullptr)
    {
        char* end = 0;
        double val = strtod(s.c_str(), &end);
        if (output != nullptr)
            *output = val;
        return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
    }
}

#endif
