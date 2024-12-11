#ifndef LOG_HELPER_HPP
#define LOG_HELPER_HPP
#include "spdlog/spdlog.h"
#include "thread/thread_utils/locker.hpp"

using loglevel = spdlog::level::level_enum;

class CLogHelper {
public:
    CLogHelper() {}
    ~CLogHelper() {}

public:
    static CLogHelper* getInstance() {
        if (m_instance == NULL) {
            locker::CAutoMutexLocker locker(&m_mutex);
            if (m_instance == NULL) {
                m_instance = new CLogHelper();
            }
        }
        return m_instance;
    }

public:
    std::string get_logger_name() { return m_logger_name; }
    bool parse_file(const std::string &json_file, std::string& logger_name, std::string& log_file_path, int &level, size_t &max_size, size_t &max_files);
    bool init_logger(const std::string& logger_name, const std::string& log_file_path, loglevel level = loglevel::info, size_t max_size = 1048576 * 10, size_t max_files = 1);
    bool init_logger(const std::string &conf_file_path);
    bool init_logger_cmd(const std::string& logger_name, loglevel level = loglevel::debug);

private:
    static locker_base::CMutexLock m_mutex;
    static CLogHelper* m_instance;

private:
    std::string m_logger_name;
    std::shared_ptr<spdlog::logger> m_logger;
};
#define LOG_HELPER() CLogHelper::getInstance()
#define LOG_INFO(msg, ...)  SPDLOG_INFO(msg, ##__VA_ARGS__);
#define LOG_DEBUG(msg, ...) SPDLOG_DEBUG(msg, ##__VA_ARGS__);
#define LOG_WARN(msg, ...)  SPDLOG_WARN(msg, ##__VA_ARGS__);
#define LOG_ERROR(msg, ...) SPDLOG_ERROR(msg, ##__VA_ARGS__);
#define LOG_CRITICAL(msg, ...) SPDLOG_CRITICAL(msg, ##__VA_ARGS__);
#endif