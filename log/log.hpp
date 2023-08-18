#ifndef LOG_LOG_HPP_
#define LOG_LOG_HPP_

#include <stdio.h>
#include <iostream>
#include <chrono>
#include <string.h>
#include "common/utils/time_utils.hpp"

#define MAX_PATH_SIZE 256

using namespace std::chrono;
static char log_file[MAX_PATH_SIZE] = "/var/log/x11.log";

inline bool set_log_path (const char *file_path) {
    if (file_path == NULL || strnlen(file_path, MAX_PATH_SIZE) == 0) {
        return false;
    }
    memset(log_file, 0, MAX_PATH_SIZE);
    strncpy(log_file, file_path, MAX_PATH_SIZE);
    return true;
}   

static char *gettime() {
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    char time_str[80];
    int len = strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    if (len == 0) {
        printf("Error formatting time\n");
        return time_str;
    }
    return time_str;
}

#define LOG_INFO(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][INFO] |%s:%d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#define LOG_ERROR(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][ERROR] |%s:%d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#define LOG_WARN(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][WARN] |%s:%d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#define LOG_DEBUG(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][DEBUG] |%s:%d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#define LOG_SUCCESS(fmt, ...) \
    do { \
        FILE *fp = fopen(log_file, "a"); \
        if (fp != NULL) { \
            fprintf(fp, "[%s][SUCCESS] |%s:%d| " fmt "\n", time_utils::FormatTimeStr(time_utils::GetTimeNow(CLOCK_REALTIME)).c_str(), __FILE__, __LINE__, ##__VA_ARGS__); \
            fclose(fp); \
        } \
    } while (0)

#endif /* LOG_LOG_H_ */