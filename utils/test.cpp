#include <iostream>
#include "file_utils.hpp"
#include "string_utils.hpp"

int main (int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <path>\n", argv[0]);
        return 1;
    }
    std::string path = argv[1];
    std::string path2 = argc > 2 ? argv[2] : "";
    std::string real_path;
    if (file_utils::get_real_path(path, real_path)) {
        printf("%s\n", real_path.c_str());
    } else {
        printf("failed\n");
    }
}