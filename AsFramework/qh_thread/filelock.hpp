#ifndef QH_THREAD_FILELOCK_HPP_
#define QH_THREAD_FILELOCK_HPP_

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace QH_THREAD {
class CFileLock {
  public:
    CFileLock() :
        m_fd_ (-1) {
    }

    ~CFileLock() {
        if (m_fd_ != -1) {
            close(m_fd_);
        }
    }
  private:
    void setFDCLOEXEC(int fd) {
        int flags = fcntl(fd, F_GETFD);
        if (flags >= 0) {
            flags |= FD_CLOEXEC;
            fcntl(fd, F_SETFD, flags);
        }
    }

  public:
    bool create(const char* file_path) {
        bool ok = false;
        m_fd_ = open(file_path, O_WRONLY|O_CREAT, S_IREAD);
        if(m_fd_ >= 0) {
            ok = true;
            setFDCLOEXEC(m_fd_);
        }

        return ok;
    }

    int lock() {
        if (m_fd_ != -1) {
            return flock(m_fd_, LOCK_EX);
        }
        return 0;
    }

    bool unlock() {
        if (m_fd_ != -1) {
            return flock(m_fd_, LOCK_UN);
        }
        return 0;
    }
  private:
    int m_fd_;
};
}
#endif /* QH_THREAD_FILELOCK_HPP_ */ 
 
