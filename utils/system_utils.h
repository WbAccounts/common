#ifndef UTILS_SYSTEM_UTILS_H_
#define UTILS_SYSTEM_UTILS_H_

#include <sys/types.h>
#include <pwd.h>
#include <string>

namespace system_utils {

std::string GetOsVersion();
bool SuperSystem(const std::string& cmd, const std::string& module_name, std::string& err_str);
int AddSyslogConfig(const std::string& conf);
int GetRandIntValue(int * value);
bool SendSig(pid_t pid, int sig);
bool SystemPopen(const std::string & strCmd, std::string & strRet);

bool GetUserName(uid_t uid,std::string& userName);
} // namespace

#endif  /* UTILS_SYSTEM_UTILS_H_ */ 
 
