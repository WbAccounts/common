#include "log_helper.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "data/json/json.hpp"
#include <fstream>

locker_base::CMutexLock CLogHelper::m_mutex;
CLogHelper* CLogHelper::m_instance = NULL;

bool CLogHelper::parse_file(const std::string &json_file, std::string& logger_name, std::string& log_file_path, int &level, size_t &max_size, size_t &max_files) 
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

bool CLogHelper::init_logger_cmd(const std::string& logger_name, loglevel level) {
    m_logger = spdlog::stdout_color_mt(logger_name);
    m_logger->set_level(level);
    m_logger_name = logger_name;
    spdlog::set_default_logger(m_logger);
    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::set_pattern("[%Y:%m:%d %X:%e %#] [line: %# ] [thread:%t] [%^%l%$] %v");
    return true;
}

bool CLogHelper::init_logger(const std::string& logger_name, const std::string& log_file_path, loglevel level, size_t max_size, size_t max_files) {
    m_logger = spdlog::rotating_logger_mt(logger_name, log_file_path, max_size, max_files);
    m_logger->set_level(level);
    m_logger_name = logger_name;
    spdlog::set_default_logger(m_logger);
    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::set_pattern("[%Y:%m:%d %X:%e] [line:%#] [thread:%t] [%^%l%$] %v");
    return true;
}

bool CLogHelper::init_logger(const std::string &conf_file_path) {
    std::string logger_name;
    std::string log_file_path;
    int level = 0;
    size_t max_size = 0;
    size_t max_files = 0;
    if (!parse_file(conf_file_path, logger_name, log_file_path, level, max_size, max_files)) {
        printf("Failed to parse json file: %s\n", conf_file_path.c_str());
        return false;
    }
    return init_logger(logger_name, log_file_path, static_cast<loglevel>(level), max_size, max_files);
}
