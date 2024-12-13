#include "string_utils.h"
#include <iostream>

int main () {
    // std::string s;
    // s = "  \nhello world  ";
    // std::cout << "ltrim:[" << string_utils::trim::ltrim(s) << "]" << std::endl;
    // s = "  \nhello world  ";
    // std::cout << "rtrim:[" << string_utils::trim::rtrim(s) << "]" << std::endl;
    // s = "  \nhello world  ";
    // std::cout << "trim:[" << string_utils::trim::trim(s) << "]" << std::endl;

    // std::string s = "123.33";
    // std::cout << string_utils::number::string_to_number<float>(s) << std::endl;

    {
        std::string s1 = "123";
        std::cout << string_utils::number::is_number(s1) << std::endl;
        std::cout << string_utils::number::is_number_a(s1) << std::endl;

        std::string s2 = "123.33";
        std::cout << string_utils::number::is_number(s2) << std::endl;
        std::cout << string_utils::number::is_number_a(s2) << std::endl;

        std::string s3 = "123a34";
        std::cout << string_utils::number::is_number(s3) << std::endl;
        std::cout << string_utils::number::is_number_a(s3) << std::endl;
    }
    return 0;
}