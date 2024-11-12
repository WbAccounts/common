#include <string>
#include <fstream>
#include <string.h>
#include "os_type.h"
#include "system_utils.h"
#include "log_helper.hpp"

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

namespace system_utils {
int get_mounts(std::vector<struct mntent> &mnts) {
    FILE *mount_table = setmntent(_PATH_MOUNTED, "r");
    if (!mount_table) {
        LOG_DEBUG("open{} failed, {}", _PATH_MOUNTED, strerror(errno));
        return -1;
    }
    struct mntent mnt_entry;
    char strings[1024];
    memset(&mnt_entry, 0x00, sizeof(mnt_entry));
    memset(&strings[0], 0x00, sizeof(strings));

    while (NULL != getmntent_r(mount_table, &mnt_entry, strings, sizeof(strings))) {
        mnts.push_back(mnt_entry);
    }
    endmntent(mount_table);
    return 0;
}

bool is_mount(const std::string &device) {
    FILE *mount_table = setmntent(_PATH_MOUNTED, "r");
    if (!mount_table) {
        LOG_DEBUG("open{} failed, {}", _PATH_MOUNTED, strerror(errno));
        return -1;
    }
    struct mntent mnt_entry;
    char strings[1024];
    memset(&mnt_entry, 0x00, sizeof(mnt_entry));
    memset(&strings[0], 0x00, sizeof(strings));

    while (NULL != getmntent_r(mount_table, &mnt_entry, strings, sizeof(strings))) {
        if (device == mnt_entry.mnt_fsname) {
            endmntent(mount_table);
            return true;
        }
    }
    endmntent(mount_table);
    return false;
}

/*  /run/systemd/seats/seat0 的 ACTIVE_UID 对应当前桌面登录用户的uid
*   
*   IS_SEAT0=1
*   CAN_MULTI_SESSION=1
*   CAN_TTY=1
*   CAN_GRAPHICAL=1
*   ACTIVE=2
*   ACTIVE_UID=1000
*   SESSIONS=2 c1
*   UIDS=1000 116
*/
bool get_login_user(std::string& uname, std::string& struid, std::string& strgid) {
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