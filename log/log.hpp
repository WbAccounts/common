#ifndef LOG_LOG_HPP_
#define LOG_LOG_HPP_

#include <stdio.h>
#include <iostream>
#include <chrono>
#include <string.h>
#include "common/time_utils/time_utils.hpp"

#define MAX_PATH_SIZE 256

static char log_file[MAX_PATH_SIZE] = "./log";

inline bool set_log_path (const char *file_path) {
    if (file_path == NULL || strnlen(file_path, MAX_PATH_SIZE) == 0) {
        return false;
    }
    memset(log_file, 0, MAX_PATH_SIZE);
    strncpy(log_file, file_path, MAX_PATH_SIZE);
    return true;
}

#define LOG_INFO(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][INFO] |%s:%4d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#define LOG_ERROR(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][ERROR] |%s:%4d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#define LOG_WARN(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][WARN] |%s:%4d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#define LOG_DEBUG(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][DEBUG] |%s:%4d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#define LOG_SUCCESS(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][SUCCESS] |%s:%4d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#endif /* LOG_LOG_H_ */
