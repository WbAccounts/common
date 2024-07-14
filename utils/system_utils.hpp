#include <string>
#include <string.h>
#include <fstream>

#if defined _WIN32

#elif defined(__linux__) || defined(__linux)
#include <pwd.h>
#if defined(__i386__)
    #define OS_X86
#elif defined(__arm__) || defined(__aarch64__)
    #define OS_ARM
#elif defined(__mips__)
    #define OS_MIPS
#elif defined(__sw_64__)
    #define OS_SW64
#elif defined(__loongarch64)
    #define OS_LOONGARCH64
#else
    #define OS_X86_64
#endif
#elif defined(__APPLE__) || defined(__MACH__)
    #define OS_MAC
#else
    #error "Unknown compiler"
#endif


namespace system_utils {



    template <typename T>
    T StringToNum(const std::string & str) {
        std::istringstream iss(str);
        T num = 0;
        iss >> num;
        return num;
    }

    template <typename T>
    std::string NumToString(T num) {
        std::ostringstream oss;
        oss << num;
        std::string str(oss.str());
        return str;
    }

    bool get_login_user(std::string& uname, std::string& struid, std::string& strgid) {
        // /run/systemd/seats/seat0 的 ACTIVE_UID对应当前桌面登录用户的uid
        // IS_SEAT0=1
        // CAN_MULTI_SESSION=1
        // CAN_TTY=1
        // CAN_GRAPHICAL=1
        // ACTIVE=2
        // ACTIVE_UID=1000
        // SESSIONS=2 c1
        // UIDS=1000 116
        std::ifstream file("/run/systemd/seats/seat0");
        if (!file) {
            return false;
        }
        std::string active_uid;
        std::string buff = "";
        while (getline(file, buff)) {
            if (buff.find("ACTIVE_UID") == std::string::npos) continue;
            active_uid = buff.substr(strlen("ACTIVE_UID") + 1);
            break;
        }
        file.close();
        uid_t uid = StringToNum<uid_t>(active_uid);
        if (active_uid.empty() || uid < 0) {
            printf("[%s] active_uid=[%s]", __FUNCTION__, active_uid.c_str());
            return false;
        }
        struct passwd * pwd = getpwuid(uid);
        if (!pwd || !pwd->pw_name) {
            printf("[%s] pwd null", __FUNCTION__);
            return false;
        }
        uname = pwd->pw_name;
        struid = NumToString<uid_t>(pwd->pw_uid);
        strgid = NumToString<gid_t>(pwd->pw_gid);
        return true;
    }
};