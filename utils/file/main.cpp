#include "file_type.hpp"
#include <iostream>

int main () {
    int a = file_utils::get_file_type("/home/wubing/code/github/FileType/fixture/fixture.eot");
    if (a == file_utils::FileType::f_eot) {
        std::cout << "eot" << std::endl;
    } else {
        std::cout << "unknown" << std::endl;
    }
    return 0;
}