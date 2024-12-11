#include "string_utils.h"
#include <iostream>

int main () {
    std::string s;
    s = "  \nhello world  ";
    std::cout << "ltrim:[" << string_utils::trim::ltrim(s) << "]" << std::endl;
    s = "  \nhello world  ";
    std::cout << "rtrim:[" << string_utils::trim::rtrim(s) << "]" << std::endl;
    s = "  \nhello world  ";
    std::cout << "trim:[" << string_utils::trim::trim(s) << "]" << std::endl;
    return 0;
}