#include <string>
#include <vector>
#include <mntent.h>

namespace system_utils {
    // mount
    int get_mounts(std::vector<struct mntent> &mnts);
    bool is_mount(const std::string &device);

    bool get_login_user(std::string& uname, std::string& struid, std::string& strgid);
};