#include "system_utils.h"
#include "log_helper.hpp"
int main () {
    log_helper::init_logger("system test", "system_test.log");

    /*
        get_mounts
    */
    // std::vector<struct mntent> mnts;
    // if (0 == system_utils::get_mounts(mnts)) {
    //     for (auto &mnt : mnts) {
    //         // 日志打印会乱码，有问题
    //         LOG_INFO("mnt_fsname: {}", mnt.mnt_fsname);
    //         LOG_INFO("mnt_dir: {}", mnt.mnt_dir);
    //         LOG_INFO("mnt_type: {}", mnt.mnt_type);
    //         LOG_INFO("mnt_opts: {}", mnt.mnt_opts);
    //         LOG_INFO("mnt_freq: {}", mnt.mnt_freq);
    //         LOG_INFO("mnt_passno: {}", mnt.mnt_passno);
    //     }
    // } else {
    //     LOG_ERROR("get_mounts failed");
    // }

    /*
        get_login_user
    */
    std::string uname;
    std::string struid;
    std::string strgid;
    if (system_utils::get_login_user(uname, struid, strgid)) {
        LOG_INFO("uname: {}", uname);
        LOG_INFO("struid: {}", struid);
        LOG_INFO("strgid: {}", strgid);
    } else {
        LOG_ERROR("get_login_user failed");
    }
    return 0;
}