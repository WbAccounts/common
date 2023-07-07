#include <iostream>
#include <vector>
#include "utils/string_utils.hpp"
#include "cron_utils.h"
#include "log/log.h"
#include "utils/time_utils.hpp"

namespace cron_utils {
#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))
enum {
    MINIUTE = 0,
    HOUR,
    DAY,
    MONTH,
    WEEKDAY
};

typedef struct CronLine {
	/* ordered by size, not in natural order. makes code smaller: */
	char cl_Dow[7];                 /* 0-6, beginning sunday */
	char cl_Mons[12];               /* 0-11 */
	char cl_Hrs[24];                /* 0-23 */
	char cl_Days[32];               /* 1-31 */
	char cl_Mins[60];               /* 0-59 */
} CronLine;

static const char *int_to_week[8] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六", "周日"};
static const char *chinese_s[] = {"分", "时", "日", "月", ""};
static const char *chinese_f[] = {"分钟", "小时", "天", "个月", ""};
static const char *month_array[12] = {
    "jan", "feb", "mar", "apr", "may", "jun",
    "jul", "aug", "sep", "oct", "nov", "dec"
};
static const char *week_array[7] = {
    "mon", "tue", "wed", "thu", "fri", "sat", "sun"
};

static int ParseField(char *ary, int modvalue, int off,
				const char *names, const char *ptr)
/* 'names' is a pointer to a set of 3-char abbreviations */
{
	const char *base = ptr;
	int n1 = -1;
	int n2 = -1;

	// this can't happen due to config_read()
	if (base == NULL || strlen(base) == 0)
		return -1;

	while (1) {
		int skip = 0;

		/* Handle numeric digit or symbol or '*' */
		if (*ptr == '*') {
			n1 = 0;  /* everything will be filled */
			n2 = modvalue - 1;
			skip = 1;
			++ptr;
		} else if (isdigit(*ptr)) {
			char *endp;
			if (n1 < 0) {
				n1 = strtol(ptr, &endp, 10) + off;
			} else {
				n2 = strtol(ptr, &endp, 10) + off;
			}
			ptr = endp; /* gcc likes temp var for &endp */
			skip = 1;
		} else if (names) {
			int i;

			for (i = 0; names[i]; i += 3) {
				/* was using strncmp before... */
				if (strncasecmp(ptr, &names[i], 3) == 0) {
					ptr += 3;
					if (n1 < 0) {
						n1 = i / 3;
					} else {
						n2 = i / 3;
					}
					skip = 1;
					break;
				}
			}
		}

		/* handle optional range '-' */
		if (skip == 0) {
			return -1;
		}
		if (*ptr == '-' && n2 < 0) {
			++ptr;
			continue;
		}

		/*
		 * collapse single-value ranges, handle skipmark, and fill
		 * in the character array appropriately.
		 */
		if (n2 < 0) {
			n2 = n1;
		}
		if (*ptr == '/') {
			char *endp;
			skip = strtol(ptr + 1, &endp, 10);
			ptr = endp; /* gcc likes temp var for &endp */
		}

		/*
		 * fill array, using a failsafe is the easiest way to prevent
		 * an endless loop
		 */
		{
			int s0 = 1;
			int failsafe = 1024;

			--n1;
			do {
				n1 = (n1 + 1) % modvalue;

				if (--s0 == 0) {
					ary[n1 % modvalue] = 1;
					s0 = skip;
				}
				if (--failsafe == 0) {
					return -1;
				}
			} while (n1 != n2);
		}
		if (*ptr != ',') {
			break;
		}
		++ptr;
		n1 = -1;
		n2 = -1;
	}

	if (*ptr) {
		return -1;
	}

    return 0;
}

static void FixDayDow(CronLine *line)
{
	unsigned i;
	int weekUsed = 0;
	int daysUsed = 0;

	for (i = 0; i < ARRAY_SIZE(line->cl_Dow); ++i) {
		if (line->cl_Dow[i] == 0) {
			weekUsed = 1;
			break;
		}
	}
	for (i = 0; i < ARRAY_SIZE(line->cl_Days); ++i) {
		if (line->cl_Days[i] == 0) {
			daysUsed = 1;
			break;
		}
	}
	if (weekUsed != daysUsed) {
		if (weekUsed)
			memset(line->cl_Days, 0, sizeof(line->cl_Days));
		else /* daysUsed */
			memset(line->cl_Dow, 0, sizeof(line->cl_Dow));
	}
}

static inline int pre_process(std::string &week_or_month)
{
    if (week_or_month.empty())
        return -1;
    for (int i = 0; i < 12; i++) {
        if (i < 7)
            string_utils::Replace(week_or_month, week_array[i], string_utils::NumToString(i + 1));
        string_utils::Replace(week_or_month, month_array[i], string_utils::NumToString(i + 1));
    }
    return 0;
}

static inline int replace_weeks(std::string &weeks)
{
    std::string p_suffix;
    if (weeks.find("-") != std::string::npos && weeks.find("/") != std::string::npos) {
        std::size_t pos = weeks.find("/");
        if (pos + 1 > weeks.size())
            return -1;
        p_suffix = "每" + weeks.substr(pos + 1) + "天";
        weeks.erase(pos, 2);
    }
    size_t pos = weeks.find("-");
    if (pos != std::string::npos)
        weeks.replace(pos, 1, "至");
    for (int i = 0; i <= 7; i++)
        string_utils::Replace(weeks, string_utils::NumToString(i), int_to_week[i]);
    weeks += p_suffix;
    return 0;
}

std::string ToReadableString(std::string mtime)
{
    // 字母表示的月份和星期替换为数字
    if (-1 == pre_process(mtime)) {
        LOG_WARN("%s: input an empty string.", __FUNCTION__);
        return "";
    }
    
    // 按字段分割字符串
    std::vector<std::string> splited;
    string_utils::Split(splited, mtime, " ");
    if (splited.size() != 5) {
        LOG_ERROR("%s: split time string failed, expected 5 words, found %d. mtime[%s]", __FUNCTION__, splited.size(), mtime.c_str());
        return "";
    }
    if (-1 == replace_weeks(splited[4])) {
        LOG_ERROR("%s: parse failed, \"day of week\" in wrong format. day_of_week[%s]", __FUNCTION__, splited[4].c_str());
        return "";
    }

    // 每个字段解析为文字描述        
    std::vector<std::string> human_readable_str;

    for (size_t i = 0; i < splited.size(); i++) {
        std::string readable_str = "";
        if (!splited[i].compare("*") || !splited[i].compare("*/1")) {
            // * 或 */1 表示每天（小时、分钟...）直接解析
            if (i != 4)
                readable_str += std::string("每") + chinese_f[i];
            human_readable_str.push_back(readable_str);
        }
        else if (splited[i].find("*/") != std::string::npos) {
            // 包含 */ 的情况解析为每xx天（分钟、小时...）
            std::size_t pos = splited[i].find("*/");
            if (pos + 2 > splited[i].size())
                continue;
            std::string p_interval = splited[i].substr(pos + 2);
            readable_str += "每" + p_interval + chinese_f[i];
            human_readable_str.push_back(readable_str);
        }
        else if (splited[i].find("-") != std::string::npos
                && splited[i].find("/") != std::string::npos) {
            // 同时含有 - 和 / 的情况（1-10/3）处理
            // 需要考虑一个字段含有多个区间的情况（1-10/3,15-21/2），逐个处理
            readable_str = splited[i];
            while (readable_str.find("/") != std::string::npos) {
                std::size_t pos = readable_str.find("/");
                if (pos + 1 > readable_str.size())
                    break;
                std::string num;
                for (std::size_t j = pos + 1; j < readable_str.size() && isdigit(readable_str[j]); ++j)
                    num += readable_str[j];
                if (1 != atoi(num.c_str()))
                    readable_str.replace(pos, num.size() + 1, chinese_s[i] + std::string("每") + num + chinese_f[i]);
                else
                    readable_str.replace(pos, 2, "时");
            }
            human_readable_str.push_back(readable_str);
        }
        else {
            readable_str += splited[i] + chinese_s[i];
            human_readable_str.push_back(readable_str);
        }
    }

    // 组合解析的文字描述字符串
    std::string output = "";
    if (splited[MONTH].compare("*") || splited[DAY].find("*") == std::string::npos)
        output += human_readable_str[MONTH];

    if (splited[DAY].compare("*") 
    || (output.empty() && splited[HOUR].find("*") == std::string::npos && !splited[WEEKDAY].compare("*")))
        output += human_readable_str[DAY];

    if (!output.empty() && splited[WEEKDAY].compare("*"))
        output += "或";
    if (output.empty() && splited[WEEKDAY].find("*") == std::string::npos)
        output += "每";
    if (splited[WEEKDAY].compare("*"))
        output += human_readable_str[WEEKDAY];

    if (splited[HOUR].compare("*")
    || (output.empty() && splited[MINIUTE].compare("*"))
    || (splited[MINIUTE].find("*") == std::string::npos))
        output += human_readable_str[HOUR];

    output += human_readable_str[MINIUTE];

    return output;
}

static int get_next_time(CronLine *line, struct tm *res_tm)
{
    int ret = -1;
    time_t secs_one_year = 31622400L;
    time_t cur = time_utils::GetTimeNow(CLOCK_REALTIME);

    for (time_t i = cur + 60; i < cur + secs_one_year; i += 60) {
        struct tm *ptm = localtime(&i);
		if (line->cl_Mins[ptm->tm_min]
		    && line->cl_Hrs[ptm->tm_hour]
			&& (line->cl_Days[ptm->tm_mday] || line->cl_Dow[ptm->tm_wday])
			&& line->cl_Mons[ptm->tm_mon]) 
        {
            *res_tm = *ptm;
            ret = 0;
            LOG_DEBUG("Time matched: %d-%d-%d %d:%d",
                      ptm->tm_year + 1900, ptm->tm_mon + 1,
                      ptm->tm_mday, ptm->tm_hour, ptm->tm_min);
            break;
        }
    }

    return ret;
}

std::string GetNextExecTime(std::string mtime)
{
    if (-1 == pre_process(mtime)) {
        LOG_WARN("%s: input an empty string.", __FUNCTION__);
        return "";
    }

    std::vector<std::string> token;
    string_utils::Split(token, mtime, " ");
    if (token.size() != 5) {
        LOG_ERROR("%s: split time string failed, expected 5 words, found %d. mtime[%s]", __FUNCTION__, token.size(), mtime.c_str());
        return "";
    }
    // ParseField 中的 day of week 取值范围为 0-6, 但是支持倒序，可以替换为 0 添加对 7 的支持
    string_utils::Replace(token[WEEKDAY], "7", "0");

    CronLine *line = (CronLine*)malloc(sizeof(CronLine));
    memset(line, 0, sizeof(CronLine));

    int i = 0;
    i += ParseField(line->cl_Mins, 60, 0, NULL, token[MINIUTE].c_str());
    i += ParseField(line->cl_Hrs, 24, 0, NULL, token[HOUR].c_str());
    i += ParseField(line->cl_Days, 32, 0, NULL, token[DAY].c_str());
    i += ParseField(line->cl_Mons, 12, -1, NULL, token[MONTH].c_str());
    i += ParseField(line->cl_Dow, 7, 0, NULL, token[WEEKDAY].c_str());
    if (i != 0) {
        LOG_ERROR("%s: Parse Cron line failed, cron express[%s]", __FUNCTION__, mtime.c_str());
        free(line);
        return "";
    }
    FixDayDow(line);

    struct tm res = {0};
    int nOpeRet = get_next_time(line, &res);
    free(line);

    if (nOpeRet != 0) {
        LOG_ERROR("Function get_next_time returned error, cron express[%s]", mtime.c_str());
        return "";
    }

    std::stringstream ss;
    ss << res.tm_year + 1900 << "年"
       << res.tm_mon + 1 << "月"
       << res.tm_mday << "日"
       << res.tm_hour << "时"
       << res.tm_min << "分";

    return ss.str();
}
} 
 
