#ifndef STRING_UTILS_TRANSFORM_HPP
#define STRING_UTILS_TRANSFORM_HPP

#include <string>
#include <algorithm>

namespace string_utils {
    namespace transform {
        std::string to_lower(std::string &s);
        std::string to_upper(std::string &s);
    };
};

namespace string_utils {
    namespace transform {
        std::string to_lower(std::string &s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        }

        std::string to_upper(std::string &s) {
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            return s;
        }
    }; // namespace transform
}; // namespace string_utils

#endif