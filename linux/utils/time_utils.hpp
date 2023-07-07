#ifndef UTILS_TIME_UTILS_HPP_
#define UTILS_TIME_UTILS_HPP_

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <cassert>
#include <string.h>
#include <string>

namespace time_utils {
const std::string kDefaultFormat = "%Y-%m-%d %H:%M:%S";
enum TimeZone { LOCALTIME = 0, GMTIM};

inline std::string FormatTimeStr(time_t time,
                                 const std::string& format = kDefaultFormat,
                                 TimeZone tz = LOCALTIME) {
    assert(!format.empty());
    struct tm time_tm;
    if (tz == LOCALTIME) {
        if (localtime_r(&time, &time_tm) == NULL) {
            return std::string();
        }
    } else {
        if (gmtime_r(&time, &time_tm) == NULL) {
            return std::string();
        }
    }

    uint64_t buf_len = 64;
    char* str_buf = new (std::nothrow) char[buf_len];
    while (str_buf != NULL &&
           strftime(str_buf, buf_len, format.c_str(), &time_tm) == 0) {
        delete[] str_buf;
        buf_len += buf_len;
        str_buf = new (std::nothrow) char[buf_len];
    }

    if (str_buf == NULL) {
        return std::string();
    }
    std::string formated_str = str_buf;
    delete[] str_buf;
    return formated_str;
}

inline std::string FormatTimeStr(const std::string& format = kDefaultFormat, TimeZone tz = LOCALTIME) {
    time_t t;
    time(&t);
    return FormatTimeStr(t, format);
}

inline time_t StrToTimestamp(const std::string& str,
                             const std::string& format = kDefaultFormat) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    if (strptime(str.c_str(), format.c_str(), &tm) == NULL) {
        return -1;
    }
    return mktime(&tm);
}

inline long GetTimeMsec(clockid_t clk_id) {
    struct timespec nsectime;
    clock_gettime(clk_id, &nsectime);
    return nsectime.tv_sec * 1000 + nsectime.tv_nsec / 1000000;
}

inline time_t GetTimeNow(clockid_t clk_id) {
    struct timespec tp;
    clock_gettime(clk_id, &tp);
    return tp.tv_sec;
}

inline time_t GetMonotonicNow() { return GetTimeNow(CLOCK_MONOTONIC); }

//将字符串的秒转化为纳秒!!
inline uint64_t StrSecToNanosec(const std::string& strSec)
{
    uint64_t nsec = 0;
    ::sscanf(strSec.c_str(),"%lu",&nsec);

    return (nsec * 1000000000);
}

} // namespace

#endif  /* UTILS_TIME_UTILS_HPP_ */
 
 
