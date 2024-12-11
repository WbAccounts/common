#include <string>

#define TRIM_CHARS "\n\t\r "

namespace string_utils {
    namespace trim {
        std::string ltrim(std::string &s, const std::string chars = TRIM_CHARS);
        std::string rtrim(std::string &s, const std::string chars = TRIM_CHARS);
        std::string trim(std::string &s, const std::string chars = TRIM_CHARS);
    };
};

namespace string_utils {
    namespace trim {
        std::string ltrim(std::string &s, const std::string chars) {
            std::string::size_type pos = s.find_first_not_of(chars); 
            if (pos == std::string::npos) {
                s.clear();
            } else {
                s.erase(0, pos);
            }
            return s;
        }

        std::string rtrim(std::string &s, const std::string chars) {
            std::string::size_type pos = s.find_last_not_of(chars);
            if (pos == std::string::npos) {
                s.clear();
            } else {
                s.erase(pos + 1);
            }
            return s;
        }

        std::string trim(std::string &s, const std::string chars) {
            ltrim(s, chars);
            rtrim(s, chars);
            return s;
        }
    };
}; // namespace string_utils