
// #include "module1.hpp"
// #include "module2.hpp"

// int main() {
//     LOG_HELPER()->init_logger("test", "test.log");
//     // LOG_HELPER()->init_logger_cmd("test");
//     printf("%d\n", __LINE__);
//     CModule1 module1;
//     CModule2 module2;
//     for (int i=0; i<10; i++) {
//         LOG_INFO("this is main thread");
//         sleep(1);
//     }
//     return 0;
// }

// #include "spdlog/spdlog.h"
// #include "spdlog/sinks/stdout_color_sinks.h"

// int main () {
//     auto console = spdlog::stdout_color_mt("console");
//     console->set_level(spdlog::level::level_enum::debug);
//     spdlog::flush_every(std::chrono::seconds(1));
//     spdlog::set_pattern("[%Y:%m:%d %X:%e %#] [line: %# ] [thread:%t] [%^%l%$] %v");

//     console->info("this is info");
//     console->debug("this is debug");
//     console->warn("this is warn");
//     console->error("this is error");
//     console->critical("this is critical");
//     return 0;
// }

#include "log_helper.h"

int main () {
    LOG_HELPER()->init_logger("test", "test.log", loglevel::debug);

    LOG_INFO("this is info");
    LOG_DEBUG("this is debug");
    LOG_WARN("this is warn");
    LOG_ERROR("this is error");
    LOG_CRITICAL("this is critical");
    return 0;
}