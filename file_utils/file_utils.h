#include <string>
#include <sys/stat.h>
#include <vector>

enum file_error {
    error_success   = 0,
    error_open_file = 1,

};

namespace file_utils {
    int read_all(const char* file_path, std::string& content);
    int read_all(const char* file_path, char **buf);     /*调用时需要将*buf初始化为空，或使用malloc申请内存空间*/
    int create_file(const char* file_path, std::string &content, int mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));

    int read_line(const char* file_path, std::vector<std::string> &lines);
};