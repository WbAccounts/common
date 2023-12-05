#include "log/log.hpp"
#include "file_utils/file_utils.h"

int main() {
    LOG_INFO("===========================================================");
    LOG_INFO("====================> START FILE UNIT <====================");
    LOG_INFO("===========================================================");

    do {
        int err = -1;
        std::string content = "This is a test!";
        err = file_utils::create_file("./test", content);
        if (err != 0) {
            LOG_ERROR("create file failed, error code: %d", err);
            break;
        }
        
        char *pstr = NULL;
        err = file_utils::read_all("/home/wubing/code/github/common/units/test", &pstr);
        if (err != 0) {
            LOG_ERROR("read file failed, error code: %d", err);
            break;
        }
        LOG_INFO("read file content char* [%s]", pstr);

    } while (false);
    return 0;
}