#include <string>
#include <unistd.h>
#include "log/log_helper.h"
#include "file/file_utils.hpp"
#include "strings/string_utils.h"

namespace proc_utils {
    std::string get_exe_name(int pid){
        char path[256] = {0};
        char buff[256] = {0};
        sprintf(path, "/proc/%d/exe", pid);
        readlink(path, buff, sizeof(buff));
        std::string str = buff;
        return string_utils::trim::trim(str);
    }

    std::string get_cwd_name(int pid){
        char path[256] = {0};
        char buff[257] = {0};
        sprintf(path, "/proc/%d/cwd",pid);
        readlink(path, buff, sizeof(buff));
        std::string str = buff;
        return string_utils::trim::trim(str);
    }

    std::string get_cmd_line(int pid){
        char filename[256]={0};
        sprintf(filename, "/proc/%d/cmdline", pid);
        std::ifstream ifs(filename);
        if (!ifs) return std::string();
        // 这里用其他的方式读取会有问题
        std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        return string_utils::trim::trim(data);
    }

    std::string get_proc_name(int pid) {
        std::string exe = get_exe_name(pid);
        size_t pos = exe.find_last_of("/");
        if (pos != std::string::npos)
            exe  = exe.substr(pos+1);
        return string_utils::trim::trim(exe);
    }
};