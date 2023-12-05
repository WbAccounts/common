#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "file_utils.h"

namespace file_utils {
    int read_all(const char* file_path, std::string& content) {
        /*Note: for performance and simplicity reasons, different fields in
        the stat structure may contain state information from different
        moments during the execution of the system call.  For example, if
        st_mode or st_uid is changed by another process by calling
        chmod(2) or chown(2), stat() might return the old st_mode
        together with the new st_uid, or the old st_uid together with the
        new st_mode.*/
        struct stat st;

        /*On success, zero is returned.  On error, -1 is returned, and
        errno is set to indicate the error.*/
        int r = stat(file_path, &st);
        if (r == -1) {
            return -1;
        }

        // Linux: 成功返回打开的文件描述符（非负整数），失败返回-1
        int fd = open(file_path, O_RDONLY);
        if (fd == -1) {
            return -1;
        }

        // calloc申请空间后会自动初始化为0，避免未定义的使用
        /*Otherwise, it shall return a null pointer and set errno to
        indicate the error.*/
        char* buf = (char*)calloc(1, st.st_size);
        if (buf == NULL) {
            return -1;
        }

        int nread = -1;
        do {
            fd = open(file_path,O_RDONLY);
            if(fd == -1) { break; }
        
            /*On success, the number of bytes read is returned (zero indicates
            end of file), and the file position is advanced by this number.*/
            nread = read(fd,buf,st.st_size);
            if(nread <= 0) { break; }
            
            content.assign(buf, nread);
        } while(false);

        if(fd >= 0) { ::close(fd); }
        if(buf) { ::free(buf); }

        return 0;
    }

    int read_all(const char* file_path, char **buf) {
        struct stat st;

        int r = stat(file_path, &st);
        if (r == -1) { return -1; }

        int fd = open(file_path, O_RDONLY);
        if (fd == -1) { return -1; }

        if (*buf == NULL) { 
            *buf = (char*)malloc(st.st_size);
            if (*buf == NULL) { return -1; }
        } else {
            *buf = (char*)realloc(*buf, st.st_size);
            if (*buf == NULL) { return -1; }
            
        }
        memset(*buf, 0, st.st_size);
        
        int nread = -1;
        do {
            fd = open(file_path,O_RDONLY);
            if(fd == -1) { break; }
        
            nread = read(fd, *buf, st.st_size);
            if(nread <= 0) { break; }
        } while(false);

        if(fd >= 0) { close(fd); }
        return 0;
    }

    int create_file(const char* file_path, std::string &content, int mode) {
        int fd = open(file_path, O_WRONLY | O_CREAT );
        if (fd == -1) { return -1; }

        /*On success, the number of bytes written is returned.  On error,
        -1 is returned, and errno is set to indicate the error.*/
        int nwrite = write(fd, content.c_str(), content.length());
        if (nwrite == -1) { return -1; }

        if(fd >= 0) { ::close(fd); }
        /*The value in errno is significant only when the return value of
        the call indicated an error (i.e., -1 from most system calls; -1
        or NULL from most library functions); a function that succeeds is
        allowed to change errno.  The value of errno is never set to zero
        by any system call or library function.*/
        return 0;
    }

    int read_line(const char* file_path, std::vector<std::string> &lines) {
        
    }
};