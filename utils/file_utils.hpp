#ifndef COMMON_FILE_UTILS_HPP
#define COMMON_FILE_UTILS_HPP

#include <string>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stack>
#include <vector>
#include <fcntl.h>
#include <time.h>
#include <fstream>
#include <dlfcn.h>
#include <dirent.h>

/*
    Functions that do not end with "l" are all penetration functions. 
    If the file is a linked file, it will search for the linked file for operation
*/

namespace file_utils {
    bool get_real_path(const std::string &path, std::string &realPath);
    bool get_real_path_l(const std::string &path, std::string &realPath);
    bool is_file(const std::string &path);
    // Hard link returns false
    bool is_link(const std::string &path);
    bool is_dir(const std::string &path);
    bool is_exist(const std::string &path);
    bool is_exist_l(const std::string &path);
    // Soft link files are all 0777
    bool get_file_mode_bin(const std::string &path, int &mode);
    bool get_file_mode_dec(const std::string &path, int &mode);
    // // Only do string processing , in:./test  out:.
    bool get_parent_dir(const std::string &path, std::string &dir);
    // path:/usr/test  in:./test  out:/usr
    bool get_real_parent_dir(const std::string &path, std::string &dir);
    bool get_real_parent_dir_l(const std::string &path, std::string &dir);
    bool get_file_size(const std::string &path, long int &size);
    // Support recursive creation
    bool make_dir(const std::string& dir, const mode_t mode = 0755);

    // On error, errno is set appropriately.
    bool move_file(const std::string &src, const std::string &dst, const mode_t mode = 0644);
    // File permissions are set according to the original file settings
    bool move_file2(const std::string &src, const std::string &dst);
    bool move_dir(const std::string &src, const std::string &dst);
    bool copy_file(const std::string &src, const std::string &dst, const mode_t mode = 0644);
    // File permissions are set according to the original file settings
    bool copy_file2(const std::string &src, const std::string &dst);
    bool copy_dir(const std::string &src, const std::string &dst);
    bool remove_file(const std::string &path);
    bool remove_dir(const std::string &path);
    // This function cannot be used to read files in the/proc directory
    // If you want, you can use the other
    bool read_file(const std::string &path, std::string &content);
    // Other
    bool read_file2(const std::string &path, std::string &content);
    bool read_file(const std::string &path, std::vector<std::string> &content);
    bool read_file_lseek(const std::string &path, std::string &content, const size_t &pos = 0, const size_t &len = -1);
    bool write_file(const std::string &path, const std::string &content, const mode_t mode = 0644);


    // 需要单独维护
    // 创建带有文件锁的文件，流程：需要挂载一个目录，这个目录具备强制锁的属性，最后将文件的软链接放到指定的路径下
    bool make_dir_mand(const std::string& dir, const mode_t mode);
    bool make_file_mand(const std::string &path, const mode_t mode);
};

namespace file_utils {
    bool get_real_path(const std::string &path, std::string &realPath) {
#if defined(__linux) || defined(_linux)
        char *tmp = ::realpath(path.c_str(), NULL);
        if (!tmp || !tmp[0]) return false;
        realPath = tmp;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool get_real_path_l(const std::string &path, std::string &realPath) {
#if defined(__linux) || defined(_linux)
        if (path.empty()) return false;
        if (!is_exist_l(path)) return false;
        if (path[0] == '/') {
            realPath = path;
            return true;
        }
        char link_path[NAME_MAX+1] = {0};
        ssize_t size = ::readlink("/proc/self/exe", link_path, NAME_MAX);
        if (size == -1) return false;

        link_path[size] = '\0';
        realPath = link_path;
        realPath.erase(realPath.find_last_of("/"));
        realPath += "/" + path;

        while (realPath.find("../") != std::string::npos) {
            // /usr/lib/../lib
            size_t pos = realPath.find("../");
            if (pos != std::string::npos) {
                if (pos == 1) {// 前面是/
                    realPath.replace(pos, strlen("../"), "");
                } else {
                    size_t lst = realPath.find_last_of("/", pos-2);
                    realPath.replace(lst+1, pos-lst-1+strlen("../"), "");
                }
            }
        }
        while (realPath.find("./") != std::string::npos) {
            size_t pos = realPath.find("./");
            if (pos != std::string::npos) {
                realPath.replace(pos, strlen("./"), "");
            }
        }
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool is_file(const std::string &path) {
#if defined(__linux) || defined(_linux)
        struct stat st;
        if (::stat(path.c_str(), &st) != 0) {
            return false;
        }
        if (!S_ISREG(st.st_mode)) {
            return false;
        }
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool is_link(const std::string &path) {
#if defined(__linux) || defined(_linux)
        struct stat st;
        if (::lstat(path.c_str(), &st) != 0) {
            return false;
        }
        if (!S_ISLNK(st.st_mode)) {
            return false;
        }
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool is_dir(const std::string &path) {
#if defined(__linux) || defined(_linux)
        struct stat st;
        if (::stat(path.c_str(), &st) != 0) {
            return false;
        }
        if (!S_ISDIR(st.st_mode)) {
            return false;
        }
        return true;
#elif defined(_WIN32)
#else
#endif
    }

    bool is_exist(const std::string &path) {
#if defined(__linux) || defined(_linux)
        struct stat st;
        if (::stat(path.c_str(), &st) != 0) {
            return false;
        }
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool is_exist_l(const std::string &path) {
#if defined(__linux) || defined(_linux)
        struct stat st;
        if (::lstat(path.c_str(), &st) != 0) {
            return false;
        }
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // 方便对权限位进行判断，可以用来设置权限位(数据是实际的权限位)
    bool get_file_mode_bin(const std::string &path, int &mode) {
#if defined(__linux) || defined(_linux)
        struct stat st;
        if (::stat(path.c_str(), &st) != 0) {
            return false;
        }
        if (st.st_mode & S_IRUSR) mode |= 0b100000000;
        if (st.st_mode & S_IWUSR) mode |= 0b010000000;
        if (st.st_mode & S_IXUSR) mode |= 0b001000000;
        // 文件所属组
        if (st.st_mode & S_IRGRP) mode |= 0b000100000;
        if (st.st_mode & S_IWGRP) mode |= 0b000010000;
        if (st.st_mode & S_IXGRP) mode |= 0b000001000;
        // 其他人
        if (st.st_mode & S_IROTH) mode |= 0b000000100;
        if (st.st_mode & S_IWOTH) mode |= 0b000000010;
        if (st.st_mode & S_IXOTH) mode |= 0b000000001;
#elif defined(_WIN32)
#else
#endif
        return true;        
    }

    // 用于打印，显示文件权限。不能用来设置(数据并不是实际的权限位)
    bool get_file_mode_dec(const std::string &path, int &mode) {
#if defined(__linux) || defined(_linux)
        struct stat st;
        if (::stat(path.c_str(), &st) != 0) {
            return false;
        }
        int u_mode = 0;
        if (st.st_mode & S_IRUSR) u_mode |= 0b100;
        if (st.st_mode & S_IWUSR) u_mode |= 0b010;
        if (st.st_mode & S_IXUSR) u_mode |= 0b001;
        int g_mode = 0;
        if (st.st_mode & S_IRGRP) g_mode |= 0b100;
        if (st.st_mode & S_IWGRP) g_mode |= 0b010;
        if (st.st_mode & S_IXGRP) g_mode |= 0b001;
        int o_mode = 0;
        if (st.st_mode & S_IROTH) o_mode |= 0b100;
        if (st.st_mode & S_IWOTH) o_mode |= 0b010;
        if (st.st_mode & S_IXOTH) o_mode |= 0b001;
        
        mode = u_mode*100 + g_mode*10 + o_mode;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // in:./test  out:.
    bool get_parent_dir(const std::string &path, std::string &dir) {
#if defined(__linux) || defined(_linux)
        bool rt = true;
        do {
            if (path.empty()) { rt = false; break; }

            char *tmp = ::strdup(path.c_str());
            if (!tmp || !tmp[0]) { rt = false; break; }

            char *dir_t = ::dirname(tmp);
            if (!dir_t || !dir_t[0])
                rt = false;
            else
                dir = dir_t;
            free(tmp);
        } while(false);
        return rt;        
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // path:/usr/test  in:./test  out:/usr
    bool get_real_parent_dir(const std::string &path, std::string &dir) {
#if defined(__linux) || defined(_linux)
        bool rt = true;
        do {
            if (path.empty()) { rt = false; break; }

            std::string realPath;
            if (!get_real_path(path, realPath)) { rt = false; break; }

            char *tmp = ::strdup(realPath.c_str());
            if (!tmp || !tmp[0]) { rt = false; break; }

            char *dir_t = ::dirname(tmp);
            if (!dir_t || !dir_t[0]) {
                rt = false;
            } else {
                dir = dir_t;
            }
            free(tmp);
        } while(false);
        return rt;        
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool get_real_parent_dir_l(const std::string &path, std::string &dir) {
#if defined(__linux) || defined(_linux)
        bool rt = true;
        do {
            if (path.empty()) { rt = false; break; }

            std::string realPath;
            if (!get_real_path_l(path, realPath)) { rt = false; break; }

            char *tmp = ::strdup(realPath.c_str());
            if (!tmp || !tmp[0]) { rt = false; break; }

            char *dir_t = ::dirname(tmp);
            if (!dir_t || !dir_t[0]) {
                rt = false;
            } else {
                dir = dir_t;
            }
            free(tmp);
        } while(false);
        return rt;        
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // /proc目录下文件不要用这个函数，获取到的大小为0
    bool get_file_size(const std::string &path, long int &size) {
#if defined(__linux) || defined(_linux)
        struct stat st;
        if (::stat(path.c_str(), &st) != 0)
            return false;
        if (!S_ISREG(st.st_mode))
            return false;
        size = st.st_size;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // 递归创建目录
    bool make_dir(const std::string& dir, const mode_t mode) {
#if defined(__linux) || defined(_linux)
        if (dir.empty()) return false;

        std::stack<std::string> Stack;
        Stack.push(dir);

        while (Stack.size() > 0) {
            std::string path = Stack.top();
            std::string parentDir;
            if (!get_parent_dir(path, parentDir)) {  
                return false;
            }
            if (is_exist_l(parentDir)) {
                // 当设置的mode大于Linux系统的umask权限时，会被umask权限覆盖，所以需要创建后chmod更改权限
                if (::mkdir(path.c_str(), mode) == -1) {
                    if (errno != EEXIST) {
                        return false;
                    }
                } else {
                    if (::chmod(path.c_str(), mode) != 0) {
                        ::rmdir(path.c_str());
                        return false;
                    }
                }
                Stack.pop();
            } else {
                Stack.push(parentDir);
            }
        }
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // 需要考虑到软链接文件的问题，如果是软链接文件，最好是获取到链接的文件，并创建一个软链
    bool move_file(const std::string &src, const std::string &dst, const mode_t mode) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        do {
            if (!is_exist_l(src) || !is_file(src) || dst.empty()) break;

            std::string dstR = dst;
            if (dst[dst.length()-1] == '/') // dir
                dstR = dst + src.substr(src.find_last_of("/")+1);

            std::string tmp;
            if (!get_parent_dir(dst, tmp)) break;
            if (!is_exist_l(tmp))
                if (!make_dir(tmp)) break;

            if (is_link(src)) {
                if (!get_real_path(src, tmp)) break;
                std::string dstTmp = dstR + "_" + std::to_string(time(NULL));
                // symlink 目标文件存在会失败
                if (::symlink(tmp.c_str(), dstTmp.c_str()) != 0) break;
                if (::rename(dstTmp.c_str(), dstR.c_str()) != 0) break;
                if (::remove(src.c_str()) != 0) break;
            } else {
                if (::rename(src.c_str(), dstR.c_str()) != 0) break;
                if (::chmod(dstR.c_str(), mode) != 0) break;
            }
            rt = true;
        } while(false);
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool move_file2(const std::string &src, const std::string &dst) {
#if defined(__linux) || defined(_linux)
        int mode = 0;
        if (get_file_mode_bin(src, mode))
            return move_file(src, dst, mode);
        return false;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // TODO:上次校验到这
    bool move_dir(const std::string &src, const std::string &dst) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        bool wh = true;
        do {
            if (!is_dir(src)) break;
            std::string destDir = dst;
            if (is_exist_l(dst))
                destDir = dst + "_" + std::to_string(time(NULL));   
            if (is_link(src)) {
                std::string realPath;
                if (!get_real_path(src, realPath)) break;
                if (symlink(realPath.c_str(), destDir.c_str()) != 0) break;
            } else {
                int mode = 0;
                if (!get_file_mode_bin(src, mode)) break;
                if (!make_dir(destDir, mode)) break;

                DIR *dir = opendir(src.c_str());
                if (!dir) break;

                while (true) {
                    errno = 0;  
                    // errno is TLS, it is a thread local storage variable and thread safe
                    dirent *node = readdir(dir);
                    if (!node || errno != 0) { wh = errno == 0 ? true : false; break; }

                    if (strncmp(node->d_name, ".", strlen(".")) == 0 ||
                        strncmp(node->d_name, "..", strlen("..")) == 0)
                        continue;
                    
                    std::string from = src + "/" + node->d_name;
                    std::string dest = destDir + "/" + node->d_name;
                    printf("move : from[%s]    to[%s]\n", from.c_str(), dest.c_str());
                    if (is_dir(from)) {
                        move_dir(from, dest);
                    } else if (is_file(from)) {
                        int mode = 0;
                        if (!get_file_mode_bin(from, mode)) { wh = false; break; }
                        move_file(from, dest, mode);
                    }
                }
                closedir(dir);
            }
            if (destDir != dst && wh) {
                if (is_link(destDir)) 
                    if (!remove_dir(dst)) break;   // 软链接如果目标目录存在，无法直接重命名
                if (rename(destDir.c_str(), dst.c_str()) != 0) wh = false;
            }
            if (!wh) { 
                remove_dir(destDir); 
                break; 
            }
            if (!remove_dir(src)) break;
            rt = true;
        } while (false);
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool copy_file(const std::string &src, const std::string &dst, const mode_t mode) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        do {
            if (!is_exist_l(src) || !is_file(src)) break;

            std::string dir;
            if (!get_parent_dir(dst, dir)) break;
                if (!is_exist_l(dir))
                    if (!make_dir(dir)) break;

            if (is_link(src)) {
                std::string dstTmp = dst + "_" + std::to_string(time(NULL));
                // symlink 目标文件存在会失败
                std::string realPath;
                if (!get_real_path(src, realPath)) break;
                if (::symlink(realPath.c_str(), dstTmp.c_str()) != 0) break;
                if (::rename(dstTmp.c_str(), dst.c_str()) != 0) break;
            } else {
                std::string content;
                if (!read_file(src, content)) break;
                if (!write_file(dst, content, mode)) break;
            }
            rt = true;
        } while (false);
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool copy_file2(const std::string &src, const std::string &dst) {
#if defined(__linux) || defined(_linux)
        int mode = 0;
        if (get_file_mode_bin(src, mode))
            return copy_file(src, dst, mode);
        return false;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool copy_dir(const std::string &src, const std::string &dst) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        do {
            if (!is_dir(src)) break;
            std::string destDir = dst;
            if (is_exist_l(dst))
                destDir = dst + "_" + std::to_string(time(NULL));   
            int mode = 0;
            if (!get_file_mode_bin(src, mode)) break;
            if (!make_dir(destDir, mode)) break;

            DIR *dir = opendir(src.c_str());
            if (!dir) break;

            bool wh = true;
            while (true) {
                errno = 0;
                dirent *node = readdir(dir);
                if (!node || errno != 0) { wh = errno == 0 ? true : false; break; }

                if (strncmp(node->d_name, ".", strlen(".")) == 0 ||
                    strncmp(node->d_name, "..", strlen("..")) == 0)
                    continue;

                std::string from = src + "/" + node->d_name;
                std::string dest = destDir + "/" + node->d_name;

                if (is_dir(from)) {
                    if (!copy_dir(from, dest)) { wh = false; break; }
                } else {
                    int mode = 0;
                    if (!get_file_mode_bin(from, mode)) { wh = false; break; }
                    if (!copy_file(from, dest, mode)) { wh = false; break; }
                }
            }
            closedir(dir);
            if (destDir != dst && wh) 
                if (rename(destDir.c_str(), dst.c_str()) != 0) break;
            if (!wh) { remove_dir(destDir); break; }
            rt = true;
        } while (false);
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool remove_file(const std::string &path) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        do {
            if (is_exist_l(path))
                if (remove(path.c_str()) != 0) break;
            rt = true; 
        } while (false);
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // 现在碰到一个问题，用ln -s 创建的软链接文件，通过remove删除时，会把指向的文件也删除，但是创建的syslink却是真的软连接。
    bool remove_dir(const std::string &path) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        do {
            DIR *dir = opendir(path.c_str());
            if (!dir) break;

            errno = 0;
            while (true) {
                dirent *node = readdir(dir);
                if (!node || errno != 0) break;

                if (strncmp(node->d_name, ".", strlen(".")) == 0 ||
                    strncmp(node->d_name, "..", strlen("..") == 0))
                    continue;

                std::string rmPath = path + "/" + node->d_name;
                printf("rmPath : %s\n", rmPath.c_str());
                if (is_dir(rmPath)) {
                    if (!remove_dir(rmPath)) return false;
                } else {
                    if (!remove_file(rmPath)) return false;
                }
            }
            closedir(dir);
            if (rmdir(path.c_str()) == -1) break;
            rt = true;
        } while (false);
        return errno != 0 ? false : rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool read_file(const std::string &path, std::string &content) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        do {
            struct stat st;
            // 软链接文件大小固定为8字节，不能用lstat获取大小
            // if (lstat(path.c_str(), &st) != 0) break;
            if (stat(path.c_str(), &st) != 0) break;

            int fd = open(path.c_str(), O_RDONLY);
            if (fd == -1) break;

            char *buf = (char *)calloc(1, st.st_size);
            if (sizeof(buf) == 0) return false;

            ssize_t size = read(fd, buf, st.st_size);
            content = buf;
            close(fd);
            free(buf);
            if (size != st.st_size) break;
            rt = true;
        } while (false);
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool read_file2(const std::string &path, std::string &content) {
        std::ifstream f(path);
        content = std::string((std::istreambuf_iterator<char>(f)),
                               std::istreambuf_iterator<char>());
        return true;
    }

    bool read_file(const std::string &path, std::vector<std::string> &content) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        do {
            if (!is_exist_l(path)) break;
            std::ifstream ifs(path);
            if (!ifs) break;
            std::string line;
            while (getline(ifs, line))
                content.push_back(line);
            rt = true;
        } while (false);
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    // TODO：未测试
    bool read_file_lseek(const std::string &path, std::string &content, const size_t &pos, const size_t &len) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        do {
            struct stat st;
            // 软链接文件大小固定为8字节，不能用lstat获取大小
            // if (lstat(path.c_str(), &st) != 0) break;
            if (stat(path.c_str(), &st) != 0) break;

            int fd = open(path.c_str(), O_RDONLY);
            if (fd == -1) break;
            if (pos != (size_t)lseek(fd, pos, SEEK_SET)) break;

            char *buf = (char *)calloc(1, st.st_size);
            if (sizeof(buf) == 0) return false;

            ssize_t size = read(fd, buf, len == -1 ? st.st_size : len);
            content = buf;
            close(fd);
            free(buf);
            if (size != st.st_size) break;
            rt = true;
        } while (false);
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }

    bool write_file(const std::string &path, const std::string &content, const mode_t mode) {
#if defined(__linux) || defined(_linux)
        bool rt = false;
        std::string tmp;
        do {
            if (!get_parent_dir(path, tmp))
                if (!is_exist_l(tmp))
                    if (!make_dir(tmp)) break;

            time_t tm = time(NULL);
            tmp = path + ".tmp_" + std::to_string(tm);
            int fd = open(tmp.c_str(), O_RDWR | O_CREAT | O_TRUNC);
            if (fd == -1) break;

            ssize_t size = write(fd, content.c_str(), content.size());
            close(fd);
            if (size != content.size()) break;
            if (rename(tmp.c_str(), path.c_str()) != 0) break;
            if (chmod(path.c_str(), mode) != 0) break;
            rt = true;
        } while(false);
        if (!tmp.empty()) remove(tmp.c_str());
        return rt;
#elif defined(_WIN32)
#else
#endif
        return true;
    }
};

#endif //COMMON_FILE_UTILS_HPP