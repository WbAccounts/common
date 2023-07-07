#include "utils/file_utils.h"
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <list>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stack>
#include <libgen.h>
#include "utils/time_utils.hpp"

namespace file_utils {
std::string GetFileName(const std::string &file_path) {
    if (file_path.empty()) {
        return std::string();
    }
    std::string::size_type last_slash_pos = file_path.rfind('/');
    if (last_slash_pos == file_path.length() - 1 ||
        last_slash_pos == std::string::npos) {
        return std::string();
    }
    std::string file_name = file_path.substr(last_slash_pos + 1);
    return file_name;
}

std::string GetFileSuffix(const std::string& file_path) {
    std::string file_name = GetFileName(file_path);
    if (file_name.empty()) {
        return std::string();
    }
    std::string::size_type last_dot_pos = file_name.rfind('.');
    if (last_dot_pos == file_path.length() - 1 || last_dot_pos == std::string::npos) {
        return std::string();
    }
    std::string suffix = file_name.substr(last_dot_pos + 1);
    return suffix;
}

std::string GetFileCompleteSuffix(const std::string& file_path) {
    std::string file_name = GetFileName(file_path);
    if (file_name.empty()) {
        return std::string();
    }
    std::string::size_type first_dot_pos = file_name.find('.');
    if (first_dot_pos == file_path.length() - 1 ||
        first_dot_pos == std::string::npos) {
        return std::string();
    }
    std::string compelte_suffix = file_name.substr(first_dot_pos + 1);
    return compelte_suffix;
}

std::string GetBaseName(const std::string &file_path) {
    std::string file_name = GetFileName(file_path);
    if (file_name.empty()) {
        return std::string();
    }
    std::string::size_type first_dot_pos = file_name.find('.');
    return file_name.substr(0, first_dot_pos);
}

std::string GetParentDir(const std::string& file_path) {
    if (file_path.empty()) {
        return std::string();
    }

    std::string trimed_path = file_path;
    string_utils::TrimRight(trimed_path, "/");
    if (file_path.empty()) {
        return std::string("/");
    }

    std::string::size_type last_slash_pos = trimed_path.rfind('/');
    if (last_slash_pos == std::string::npos) {
        return std::string();
    }

    std::string parent_dir = "/";
    //如果last_slash_post为0，表示对应的路径是在根(/)路径下；
    //比如: /a.txt
    if(last_slash_pos > 0) {
        parent_dir = trimed_path.substr(0, last_slash_pos);
    }
    
    return string_utils::TrimRight(parent_dir);
}

std::string GetParentDir2(const std::string& file_path)
{
    std::string pdir = "";
    if(!file_path.empty()) {
       GetDirName(file_path.c_str(),pdir);
    }

    return pdir;
}

bool FollowLink(const std::string &link, std::string &real) {
    char *resolved = realpath(link.c_str(), NULL);
    if (resolved == NULL) {
        return false;
    }
    real = std::string(resolved);
    free(resolved);
    return true;
}

bool FollowLink2(const char* link, std::string &real) {
    char *resolved = realpath(link, NULL);
    if (resolved == NULL) {
        return false;
    }
    real = std::string(resolved);
    free(resolved);
    return true;
}


bool IsFile(const std::string &file_path, FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return false;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return IsFile(real_path);
            } else {
                return false;
            }
        } else {
            return true;
        }
    }
    if (S_ISREG(lsb.st_mode)) {
        return true;
    } else {
        return false;
    }
}

bool IsDir(const std::string &file_path, FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return false;
    }

    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return IsDir(real_path);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    if (S_ISDIR(lsb.st_mode)) {
        return true;
    } else {
        return false;
    }
}

bool IsSymLink(const std::string &file_path, bool must_effective) {
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return false;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (must_effective) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }
    }
    return false;
}

bool IsExist(const std::string &file_path, FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return false;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }
    }
    return true;
}

int64_t GetFileSize(const std::string &file_path,
                    FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return -1;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return GetFileSize(real_path);
            } else {
                return -1;
            }
        } else {
            return lsb.st_size;
        }
    }
    if (S_ISREG(lsb.st_mode)) {
        return lsb.st_size;
    }
    return -1;
}

bool MakeDirs(const std::string& dir, const mode_t mode) {
    if (dir.empty()) {
        return false;
    }

    std::list<std::string> lst;
    lst.push_back(dir);

    while (!lst.empty()) {
        std::string dir = lst.back();
        if (IsExist(dir)) {
            lst.pop_back();
        } else {
            std::string parent_dir = GetParentDir(dir);
            if (IsExist(parent_dir) || parent_dir.empty()) {
                if (::mkdir(dir.c_str(), mode) == -1) {
                    if (errno != EEXIST) {
                        return false;
                    }
                }
                lst.pop_back();
            } else {
                lst.push_back(parent_dir);
            }
        }
    }
    return true;
}

bool IsReadable(const std::string &file_path) {
    return access(file_path.c_str(), R_OK) == 0;
}

bool IsWritable(const std::string &file_path) {
    return access(file_path.c_str(), W_OK) == 0;
}

bool IsExecuteble(const std::string& file_path) {
    return access(file_path.c_str(), X_OK) == 0;
}

uid_t GetOwnerId(const std::string &file_path,
                 FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return -1;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return GetOwnerId(real_path);
            } else {
                return (uid_t) -2;
            }
        } else {
            return lsb.st_uid;
        }
    }
    return lsb.st_uid;
}

std::string GetOwner(const std::string &file_path,
                     FOLLOW_LINK_TYPE follow_link_type) {
    std::string name;
    uid_t uid = GetOwnerId(file_path, follow_link_type);
    if (uid == static_cast<uid_t>(-2)) return name;

    struct passwd pd;
    struct passwd *result;

    char *buffer;
    size_t buffer_size = sysconf(_SC_GETPW_R_SIZE_MAX);
    buffer = new (std::nothrow) char[buffer_size];
    if (buffer == NULL) {
        return name;
    }
    if (getpwuid_r(uid, &pd, buffer, buffer_size, &result) == 0) {
        name = std::string(pd.pw_name);
    }
    delete[] buffer;
    return name;
}

time_t LastModified(const std::string &file_path,
                    FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return -1;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return LastModified(real_path);
            } else {
                return -2;
            }
        } else {
            return lsb.st_mtime;
        }
    }
    return lsb.st_mtime;
}

std::string FileTimeToStr(time_t time) {
    return time_utils::FormatTimeStr(time, "%Y-%m-%d %H:%M:%S");
}

std::string LastModifiedStr(const std::string& file_path,
                            FOLLOW_LINK_TYPE follow_link_type) {
    time_t last_modified = LastModified(file_path, follow_link_type);
    if (last_modified == -2) {
        return std::string();
    }
    return FileTimeToStr(last_modified);
}

long GetActiveFileNumber() {
    long open_file_counter = 0;
    char buf[256] = {0};
    FILE *fp = fopen("/proc/sys/fs/file-nr", "r");
    if (fp == NULL) {
        return 0;
    }

    if (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
        sscanf(buf, "%ld", &open_file_counter);
    }
    fclose(fp);

    return open_file_counter;
}

bool CopyFile(const std::string& from, const std::string& to) {
    std::string to_parent_dir = GetParentDir(to);
    if (!to_parent_dir.empty() && !IsDir(to_parent_dir)) {
        if (!MakeDirs(to_parent_dir)) {
            return false;
        }
    }
    std::ifstream in;
    in.open(from.c_str());
    if (!in) return false;
    std::ofstream out;
    out.open(to.c_str());
    if (!out) return false;
    out << in.rdbuf();
    in.close();
    out.close();
    return true;
}

bool CopyFile(const std::string& from, const std::string& to, mode_t mode) {
    if (false == CopyFile(from, to)) {
        return false;
    }
    return (0 == chmod(to.c_str(), mode) ? true : false);
}

bool MoveFile(const std::string& from, const std::string& to) {
    std::string to_parent_dir = GetParentDir(to);
    if (!to_parent_dir.empty() && !IsDir(to_parent_dir)) {
        if (!MakeDirs(to_parent_dir)) {
            return false;
        }
    }
    if (0 != rename(from.c_str(), to.c_str())) return false;
    return true;
}

bool MoveFile(const std::string& from, const std::string& to, mode_t mode) {
    if (false == MoveFile(from, to)) {
        return false;
    }
    return (0 == chmod(to.c_str(), mode) ? true : false);
}

bool MoveDir(const std::string& from, const std::string& to) {
    DIR *dirs = NULL;
    if (!(dirs = opendir(from.c_str()))) {
        return false;
    }
    int dirent_len = offsetof(struct dirent, d_name) + GetPathMaxSize(from) + 1;
    struct dirent *file = (struct dirent *)malloc(dirent_len);
    if (file == NULL) {
        closedir(dirs);
        return false;
    }
    memset((void *)file, 0, dirent_len);
    struct dirent *result = NULL;
    bool rtn = true;
    while (readdir_r(dirs, file, &result) == 0 && result != NULL) {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
            continue;
        std::string from_full_path = from + "/" + file->d_name;
        std::string to_full_path = to + "/" + file->d_name;
        memset((void *)file, 0, dirent_len);
        struct stat st;
        if (lstat(from_full_path.c_str(), &st) == -1)
            continue;
        if (S_ISDIR(st.st_mode)) {
            rtn = MoveDir(from_full_path, to_full_path);
        } else if (S_ISREG(st.st_mode)) {
            rtn = MoveFile(from_full_path, to_full_path);
        } else {
            continue;
        }
        if (false == rtn) {
            break;
        }
    }
    closedir(dirs);
    free(file);
    RemoveDirs(from);
    return rtn;
}

bool CopyDir(const std::string& from, const std::string& to, mode_t mode) {
    DIR *dirs = NULL;
    if (!(dirs = opendir(from.c_str()))) {
        return false;
    }
    int dirent_len = offsetof(struct dirent, d_name) + GetPathMaxSize(from) + 1;
    struct dirent *file = (struct dirent *)malloc(dirent_len);
    if (file == NULL) {
        closedir(dirs);
        return false;
    }
    memset((void *)file, 0, dirent_len);
    struct dirent *result = NULL;
    bool rtn = true;
    while (readdir_r(dirs, file, &result) == 0 && result != NULL) {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
            continue;
        std::string from_full_path = from + "/" + file->d_name;
        std::string to_full_path = to + "/" + file->d_name;
        memset((void *)file, 0, dirent_len);
        struct stat st;
        if (lstat(from_full_path.c_str(), &st) == -1)
            continue;
        if (S_ISDIR(st.st_mode)) {
            rtn = CopyDir(from_full_path, to_full_path, mode);
        } else if (S_ISREG(st.st_mode)) {
            rtn = CopyFile(from_full_path, to_full_path, mode);
        } else {
            continue;
        }
        if (false == rtn) {
            break;
        }
    }
    closedir(dirs);
    free(file);
    return rtn;
}

bool RemoveDirs(const std::string& dir, FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    if (lstat(dir.c_str(), &lsb) != 0) {
        return true;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(dir, real_path);
            if (ok) {
                return RemoveDirs(real_path);
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else if (S_ISDIR(lsb.st_mode)) {
        DIR* dirp = opendir(dir.c_str());
        if (!dirp) {
           return false;
        }
        struct dirent *p_dirent;
        while((p_dirent = readdir(dirp)) != NULL) {
            if(strcmp(p_dirent->d_name, ".") == 0 || strcmp(p_dirent->d_name, "..") == 0) {
                continue;
            }
            struct stat st;
            std::string sub_path = dir + '/' + p_dirent->d_name;
            if (lstat(sub_path.c_str(), &st) == -1) {
                continue;
            }
            if (S_ISDIR(st.st_mode)) {
                if (RemoveDirs(sub_path) == false) {
                    closedir(dirp);
                    return false;
                }
            } else if (S_ISREG(st.st_mode)) {
                RemoveFile(sub_path);
            } else {
                continue;
            }
        }
        if (rmdir(dir.c_str()) == -1) {
            closedir(dirp);
            return false;
        }
        closedir(dirp);
    }
    return true;
}

namespace {
bool DoRemoveFile(const std::string& file_path) {
    if (0 != remove(file_path.c_str())) return false;
    return true;
}
}

bool RemoveFile(const std::string& file_path,
                FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return true;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return RemoveFile(real_path);
            } else {
                return false;
            }
        } else {
            return DoRemoveFile(file_path);
        }
    } else if (S_ISREG(lsb.st_mode)) {
        return DoRemoveFile(file_path);
    }
    return false;
}

FileContentSmartPtr GetFileContent(const std::string& file_path, size_t& size,
                                   FOLLOW_LINK_TYPE follow_link_type) {
    struct stat lsb;
    size = 0;
    FileContentSmartPtr null_ptr ;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return null_ptr;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return GetFileContent(real_path, size);
            } else {
                return null_ptr;
            }
        } else {
            return null_ptr;
        }
    } else if (S_ISREG(lsb.st_mode)) {
        int fd = open(file_path.c_str(), O_RDONLY | O_NOFOLLOW);
        if (fd < 0) {
            return null_ptr;
        }
        do {
            void* content_buf = malloc(lsb.st_size);
            if (content_buf == NULL) {
                break;
            }
            FileContentSmartPtr ptr(content_buf, free);
            ssize_t read_size = read(fd, content_buf, lsb.st_size);
            if (read_size != lsb.st_size) {
                break;
            }
            size = read_size;
            close(fd);
            return ptr;
        } while(false);
        close(fd);
        return null_ptr;
    }
    return null_ptr;
}

bool GetFileContent(const std::string& file_path, int64_t file_size, char* buf,
                    FOLLOW_LINK_TYPE follow_link_type) {
    if (file_path.empty() || file_size <= 0 || buf == NULL) return false;
    struct stat lsb;
    if (lstat(file_path.c_str(), &lsb) != 0) {
        return false;
    }
    if (S_ISLNK(lsb.st_mode)) {
        if (follow_link_type == FOLLOW_LINK) {
            std::string real_path;
            bool ok = FollowLink(file_path, real_path);
            if (ok) {
                return GetFileContent(real_path, file_size, buf);
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else if (S_ISREG(lsb.st_mode)) {
        int fd = open(file_path.c_str(), O_RDONLY | O_NOFOLLOW);
        if (fd < 0) {
            return false;
        }
        bool ret = true;
        long offset = 0;
        long remainningSize = file_size;
        while (offset < file_size) {
            static const size_t nMaxSizePerRound = 2 << 20;  // 2M
            size_t nSize2Read = (remainningSize >= (long)nMaxSizePerRound)
                                    ? nMaxSizePerRound
                                    : remainningSize;
            size_t nSizeRead = ::read(fd, buf + offset, nSize2Read);
            if (nSizeRead != nSize2Read) {
                ret = false;
                break;
            }
            remainningSize -= nSizeRead;
            offset += nSizeRead;
        }
        close(fd);
        return ret;
    }
    return false;
}

int GetPathMaxSize(const std::string &path) {
    int path_max;
#ifdef PATH_MAX
    (void)path;
    path_max = PATH_MAX;
#else
    path_max = pathconf(path, _PC_PATH_MAX);
    if (path_max <= 0)
        path_max = 4096;
#endif

    return path_max;
}

void SetFDCLOEXEC(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags >= 0) {
        flags |= FD_CLOEXEC;
        fcntl(fd, F_SETFD, flags);
    }
}

void SetFDNONBLOCK(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags >= 0) {
        flags |= O_NONBLOCK;
        fcntl(fd, F_SETFL, flags);
    }
}

std::string MakeTempDir(const std::string& strTemplate) {
    if (strTemplate.empty()) {
        return "";
    }

    std::string strTempDir;
    char *pTemplate = strdup(strTemplate.c_str());
    if (pTemplate) {
        char *pDir = mkdtemp(pTemplate);
        if (pDir) {
            strTempDir = pDir;
        } else {
            std::ostringstream oss;
            oss << time(NULL);
            strTempDir = strTemplate + oss.str();
            if (!MakeDirs(strTempDir)) {
                strTempDir.clear();
            }
        }
        free(pTemplate);
    }
    return strTempDir;
}

static std::string getRealPathAndStat(const std::string& cur_dir,
                        const struct dirent* dent,struct stat& st)
{
     char* rpath = NULL;
    std::string fullPath = cur_dir + "/" 
                        + dent->d_name;
    
    do {
        char* rpath = ::realpath(fullPath.c_str(),NULL);
        if(!rpath) { fullPath = ""; break; }

        int rc = ::stat(rpath,&st);
        if(rc < 0) { fullPath = ""; break; }

        fullPath = rpath;
    } while(false);

    if(rpath) { ::free(rpath); }

    return fullPath;
}

static void procDent(const struct dirent* dent,const std::string& cur_dir,
            std::stack<std::string>& dirs,
            bool (*cb)(const char* fpath,const struct stat* st,void* ctx),void* ctx)
{
    std::string fullPath;

    do {
        struct stat st;
        if(!strcmp(dent->d_name,".") ||
           !strcmp(dent->d_name,".."))
        {
            break;
        }

        fullPath = getRealPathAndStat(cur_dir,dent,st);
        if(fullPath.empty()) { break; }

        //是目录则加入目录队列，是文件则进行扫描
        if (S_ISDIR(st.st_mode)) { 
            dirs.push(fullPath); 
            break;
        }

        if(!cb(fullPath.c_str(),&st,ctx)) {
            break;
        }
    }while(false);
}

//这个枚举目录的函数处理不了自己软链接到自己的情况，并且无法去重
bool EnumDir(const std::string &dir,
        bool (*cb)(const char* fpath,const struct stat* st,void* ctx),
        void* ctx)
{
    bool ok = true;
    DIR * dirp = NULL;
    struct dirent* result = NULL;
    std::stack<std::string> dirs;

    #ifdef PATH_MAX
        int path_max = PATH_MAX;
    #else
        int path_max = pathconf (dir, _PC_PATH_MAX);
        if (path_max <= 0)
            path_max = 4096;
    #endif

    int dent_len = offsetof(struct dirent, d_name) + path_max + 1;
    struct dirent* dent = (struct dirent*)calloc(1,dent_len);
    if (dent == NULL) { return false; }

    dirs.push(dir);

    while (!dirs.empty()){
        std::string cur_dir = dirs.top();
        dirs.pop();
        if((cur_dir == ".") || 
            (cur_dir == "..")) 
        {
            continue;
        }

        // 打开目录
       dirp = opendir(cur_dir.c_str());
       if (!dirp) { continue; }

        // 遍历目录中的每一个文件和目录
        while(readdir_r(dirp,dent, &result) == 0 && result != NULL){
            procDent(dent,cur_dir,dirs,cb,ctx);
            memset((void*)dent, 0, dent_len);
        }
        closedir(dirp);
    }
    free(dent);
    return ok;
}

bool GetDirName(const char* fpath,
                std::string& dname)
{
    bool ok = false;
    do {
        if(!fpath || !fpath[0]) 
        { break; }

        char* sdup = ::strdup(fpath);
        if(!sdup) { break; }

        char* pdir = ::dirname(sdup);
        if(pdir && pdir[0]) {
            dname = pdir;
            ok = true;
        }
        ::free(sdup);
    } while(false);

    return ok;
}

bool GetBaseName2(const char* fpath,
                std::string& bname)
{
    bool ok = false;
    do {
        if(!fpath || !fpath[0]) 
        { break; }

        char* sdup = ::strdup(fpath);
        if(!sdup) { break; }

        char* p = ::basename(sdup);
        if(p && p[0]) {
            bname = p;
            ok = true;
        }
        ::free(sdup);
    } while(false);

    return ok;
}

//返回错误码
static int doOverWriteFile(const char* fpath,
                const std::string& data,mode_t mode)
{
    int err = 0;
    int oflags = O_CREAT | O_RDWR | O_TRUNC;
    do {
        int fd = ::open(fpath,oflags,mode);
        if(-1 == fd) { err = errno; break; }
        
        ssize_t nw = ::write(fd,data.data(),
                            data.size());
        ::close(fd);
        if(nw < 0) { err = errno; break; }

        if((size_t)nw != data.size()) {
            err = errno;
            break;
        }
    } while(false);

    return err;
}

int OverWriteFile(const char* fpath,
        const std::string& data,
        mode_t mode)
{
    int err = 0;
    std::string ftmp;
    
    do {
        std::string dname,bname;
        bool ok = GetDirName(fpath,dname);
        if(!ok) { err = errno; break; }

        ok = GetBaseName2(fpath,bname);
        if(!ok) { err = errno; break; }

        std::string ftmp = dname + 
                    "/.tmpxxx" + bname;
        
        err = doOverWriteFile(ftmp.c_str(),
                            data,mode);
        if(err) { break; }

        if(0 != ::rename(ftmp.c_str(),fpath)) {
            err = errno;
        }
        if(err) { break; }

        if(0 != ::chmod(fpath, mode)) {
            err = errno;
        }
    } while(false);

    //由于rename也可能失败，所以无论最后成功还是失败都尝试删除一下临时文件
    if(!ftmp.empty()) { ::remove(ftmp.c_str()); }

    return err;
}

int ReadAll(const char* fpath,
            std::string& data)
{
    int fd = -1;
	struct stat st;
    ssize_t nread = -1;

	int rc = ::stat(fpath,&st);
    if(rc < 0) { return (int)nread; }
	
	char* buf = (char*)calloc(1,st.st_size);
	if(!buf) { return (int)nread; }
	
	do {
		fd = ::open(fpath,O_RDONLY);
		if(fd < 0) { break; }
	
		nread = ::read(fd,buf,st.st_size);
		if(nread <= 0) { break; }
		
		data.assign(buf,nread);	
	} while(false);	

	if(fd >= 0) { ::close(fd); }
	if(buf) { ::free(buf); }

	return (int)(nread);
}

bool listFilesInDirectory(const std::string& path, std::vector<std::string>& files) {
    struct dirent **namelist;
    int count = scandir(path.c_str(), &namelist, NULL, NULL);
    if (count < 0) {
        return false;
    }

    for (int i = 0; i < count; ++i) {
        if (strcmp(namelist[i]->d_name, ".") != 0 && strcmp(namelist[i]->d_name, "..") != 0) {
            files.push_back(std::string(namelist[i]->d_name));
        }
        free(namelist[i]);
    }

    free(namelist);

    return true;
}

}
 
 
