#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "data/json/json.hpp"
#include <fstream>

using loglevel = spdlog::level::level_enum;

// #define SPDLOG_LEVEL_TRACE 0
// #define SPDLOG_LEVEL_DEBUG 1
// #define SPDLOG_LEVEL_INFO 2
// #define SPDLOG_LEVEL_WARN 3
// #define SPDLOG_LEVEL_ERROR 4
// #define SPDLOG_LEVEL_CRITICAL 5
// #define SPDLOG_LEVEL_OFF 6

std::shared_ptr<spdlog::logger> m_logger;
std::string logger_name;

namespace log_helper {
namespace json_helper {
    bool parse_file(const std::string &json_file, std::string& logger_name, std::string& log_file_path, int &level, size_t &max_size, size_t &max_files) 
    {
        std::ifstream f(json_file);
        if (!f) {
            return false;
        }
        nlohmann::json data = nlohmann::json::parse(f);
        data.at("logger_name").get_to(logger_name);
        data.at("log_level").get_to(level);
        data.at("log_file").get_to(log_file_path);
        data.at("log_max_size").get_to(max_size);
        data.at("log_max_files").get_to(max_files);
        return true;
    }
};
    bool init_logger(const std::string& logger_name, const std::string& log_file_path, loglevel level = loglevel::info, size_t max_size = 1048576 * 5, size_t max_files = 3) {
        m_logger = spdlog::rotating_logger_mt(logger_name, log_file_path, max_size, max_files);
        m_logger->set_level(level);
        return true;
    }

    bool init_logger(const std::string &conf_file_path) {
        std::string logger_name;
        std::string log_file_path;
        int level = 0;
        size_t max_size = 0;
        size_t max_files = 0;
        if (!json_helper::parse_file(conf_file_path, logger_name, log_file_path, level, max_size, max_files)) {
            printf("Failed to parse json file: %s\n", conf_file_path.c_str());
            return false;
        }
        return init_logger(logger_name, log_file_path, static_cast<loglevel>(level), max_size, max_files);
    }

    
};

#define LOG_INFO(msg, ...)  m_logger->info(msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) m_logger->debug(msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)  m_logger->warn(msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) m_logger->error(msg, ##__VA_ARGS__)
