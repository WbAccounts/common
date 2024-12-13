#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>

namespace string_utils {
    namespace number
    {
        bool is_number(const std::string &s);   // 支持多进制及浮点
        bool is_number_a(const std::string &s); // 仅十进制

        template<typename T>
        T string_to_number(const std::string &s);

        int bin_2_int(const std::string& s);
        int oct_2_int(const std::string& s);
        int dec_2_int(const std::string& s);
        int hex_2_int(const std::string& s);

        std::string to_bin_string(const int &i);
        std::string to_oct_string(const int &i);
        std::string to_dec_string(const int &i);
        std::string to_hex_string(const int &i);

        std::string bin_2_oct(const std::string &s);
        std::string bin_2_dec(const std::string &s);
        std::string bin_2_hex(const std::string &s);
        std::string oct_2_bin(const std::string &s);
        std::string oct_2_dec(const std::string &s);
        std::string oct_2_hex(const std::string &s);
        std::string dec_2_bin(const std::string &s);
        std::string dec_2_oct(const std::string &s);
        std::string dec_2_hex(const std::string &s);
        std::string hex_2_bin(const std::string &s);
        std::string hex_2_oct(const std::string &s);
        std::string hex_2_dec(const std::string &s);
    };
}; 

namespace string_utils {
    namespace number
    {
        bool is_number(const std::string &s) {
            std::istringstream ss(s);
            double i;
            ss >> i;
            return !ss.fail();
        }

        bool is_number_a(const std::string &s) {
            return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
        }

        template<typename T>
        T string_to_number(const std::string &s) {
            std::istringstream ss(s);
            T i;
            ss >> i;
            return i;
        }

        int bin_2_int(const std::string& s) {
            int decimal = 0;
            int length = s.length();
            for (int i = 0; i < length; ++i) {
                if (s[i] == '1') {
                    decimal += static_cast<int>(pow(2, length - i - 1));
                }
            }
            return decimal;
        }

        int dec_2_int(const std::string& s) {
            int i;
            ::sscanf(s.c_str(), "%d", &i);
            return i;
        }

        int oct_2_int(const std::string& s) {
            int i;
            ::sscanf(s.c_str(), "%o", &i);
            return i;
        }

        int hex_2_int(const std::string& s) {
            int i;
            ::sscanf(s.c_str(), "%x", &i);
            return i;
        }

        std::string to_bin_string(const int &i) {
            int t = i;
            std::string binary = "";
            while (t > 0) {
                binary = char('0' + t % 2) + binary; // 计算当前位并添加到字符串前面
                t /= 2; // 移动到下一个位
            }
            return binary.empty() ? "0" : binary; // 处理输入为0的情况
        }

        std::string to_dec_string(const int &i) {
            char s[128] = {0};
            ::sprintf(s, "%d", i);
            return std::string(s);
        }

        std::string to_oct_string(const int &i) {
            char s[128] = {0};
            ::sprintf(s, "%o", i);
            return std::string(s);
        }

        std::string to_hex_string(const int &i) {
            char s[128] = {0};
            ::sprintf(s, "%x", i);
            return std::string(s);
        }

        std::string bin_2_oct(const std::string &s) {
            return to_oct_string(bin_2_int(s));
        }

        std::string bin_2_dec(const std::string &s) {
            return to_dec_string(bin_2_int(s));
        }

        std::string bin_2_hex(const std::string &s) {
            return to_hex_string(bin_2_int(s));
        }

        std::string oct_2_bin(const std::string &s) {
            return to_bin_string(oct_2_int(s));
        }
        
        std::string oct_2_dec(const std::string &s) {
            return to_dec_string(oct_2_int(s));
        }

        std::string oct_2_hex(const std::string &s) {
            return to_hex_string(oct_2_int(s));
        }

        std::string dec_2_bin(const std::string &s) {
            return to_bin_string(dec_2_int(s));
        }

        std::string dec_2_oct(const std::string &s) {
            return to_oct_string(dec_2_int(s));
        }

        std::string dec_2_hex(const std::string &s) {
            return to_hex_string(dec_2_int(s));
        }

        std::string hex_2_bin(const std::string &s) {
            return to_bin_string(hex_2_int(s));
        }

        std::string hex_2_oct(const std::string &s) {
            return to_oct_string(hex_2_int(s));
        }

        std::string hex_2_dec(const std::string &s) {
            return to_dec_string(hex_2_int(s));
        }
    }; // namespace number
}; // namespace string_utils