#include "log_helper.hpp"

int main () {
    log_helper::init_logger("test", "test.log");
    std::string msg = "你好，你奶奶个腿的";
    LOG_INFO("{}", msg);
    return 0;
}