#include <sys/fanotify.h>
#include <fcntl.h>
#include <string.h>

#include "file_block.h"
#include "system/system_utils.h"

#define IS_GREATER(x) ((x) > 0)

CFileBlock::CFileBlock() {

}

CFileBlock::~CFileBlock() {

}

bool CFileBlock::init() {
    std::string kernel_ = system_utils::get_kernel_release();
    unsigned int flags = FAN_CLASS_PRE_CONTENT | // 文件被访问前通知
                         FAN_UNLIMITED_QUEUE |   // 移除事件中事件数量的限制
                         FAN_UNLIMITED_MARKS;    // 取消每个用户的fanotify标记数量限制
    if (IS_GREATER(system_utils::compare_version(kernel_, "5.1"))) {
        flags |= FAN_REPORT_FID; // 5.1版本之后，包含关于底层文件系统的附加信息
    }
    if (IS_GREATER(system_utils::compare_version(kernel_, "5.9"))) {
        flags |= FAN_REPORT_DIR_FID; // 5.9版本之后，包含关于目录的附加信息
    }

    m_fd = fanotify_init(flags, O_RDWR);
    if (m_fd == -1) {
        return false;
    } else {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_fd, &rfds);
    }

    fanotify_mark(m_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_OPEN | FAN_CLOSE | FAN_ACCESS_PERM | FAN_MODIFY | FAN_EVENT_ON_CHILD | FAN_DELETE, AT_FDCWD, "/home/wubing/code/github/common/system/test/wubing");

    m_thread_monitor.set_thread_func(std::tr1::bind(&CFileBlock::thread_monitor, this, std::tr1::placeholders::_1));
    m_thread_monitor.run();

    m_thread_deal.set_thread_func(std::tr1::bind(&CFileBlock::thread_deal, this, std::tr1::placeholders::_1));
    m_thread_deal.run();
    m_thread_deal.pause();
    return true;
}

bool CFileBlock::subscribe(const std::string& id, const file_block::SubInfo &sub) {
    

    return true;
}

void CFileBlock::update_ev_count(int event, bool isAdd) {
    locker::CAutoMutexLocker locker(&m_sub_mutex);
    if (isAdd) {
        if (m_ev_count.find(event) == m_ev_count.end()) {
            m_ev_count[event] = 0;
        }
        m_ev_count[event]++;
    } else {
        if (m_ev_count.find(event) != m_ev_count.end()) {
            m_ev_count[event]--;
            if (m_ev_count[event] == 0) {
                m_ev_count.erase(event);
            }
        }

    }

    // if (m_ev_count.find(event) == m_ev_count.end()) {
    //     m_ev_count[event] = 0;
    // }
    // m_ev_count[event] += op;
}

void* CFileBlock::thread_monitor(void* arg) {
    fd_set rfds;
    while (!m_thread_monitor.isQuit()) {
        if (m_thread_monitor.isPause()) {
            m_thread_monitor.wait();
        } else {
            // do something
            // FD_ZERO(&rfds);
            // FD_SET(m_fd, &rfds);

            struct timeval timeo = {.tv_sec = 0, .tv_usec = 500000};
            char buf[4096] = {0};
            int ret = select(m_fd + 1, &rfds, NULL, NULL, &timeo);
            if (ret < 0) {
                close(m_fd);
                m_fd = -1;
                break;
            } else if (ret == 0) {
                continue;
            }
            struct fanotify_event_metadata* metadata =
            reinterpret_cast<struct fanotify_event_metadata*>(buf);
            ssize_t len = read(m_fd, buf, sizeof(buf));

            if (len == -1 && errno == EAGAIN) {
                continue;
            }

            if (len == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            struct fanotify_response resp;
            // 遍历返回的变更事件
            for (; FAN_EVENT_OK(metadata, len);
                metadata = FAN_EVENT_NEXT(metadata, len)) {
                printf("fd=%d pid=%d ", metadata->fd, metadata->pid);
                if (metadata->mask & FAN_ACCESS) {
                    printf("event=read ");
                } else if (metadata->mask & FAN_MODIFY) {
                    printf("event=write ");
                } else if(metadata->mask & FAN_OPEN_PERM) {
                    printf("event=open_perm response=allow ");
                    // 收到open调用，返回允许
                    resp.fd = metadata->fd;
                    resp.response = FAN_ALLOW;
                    if(write(m_fd, (char*)&resp, sizeof(resp)) < 0) {
                        perror("write reponse");
                        close(metadata->fd);
                        continue;
                    }
                } else if(metadata->mask & FAN_ACCESS_PERM) {
                    printf("event=access_perm response=deny ");
                    // 收到read调用，返回禁止
                    resp.fd = metadata->fd;
                    resp.response = FAN_DENY;
                    if(write(m_fd, (char*)&resp, sizeof(resp)) < 0) {
                        perror("write reponse");
                        close(metadata->fd);
                        continue;
                    }
                }

                // 根据返回事件中的fd读取操作的文件路径
                char path[1024] = {0};
                char flink[1024] = {0};
                sprintf(flink, "/proc/self/fd/%d", metadata->fd);
                if (readlink(flink, path, sizeof(path)) < 0) {
                    printf("readlink %s failed: %s\n", flink, strerror(errno));
                    close(metadata->fd);
                    printf("\n");
                    continue;
                }
                printf("path=%s\n", path);
                close(metadata->fd);
            }
        }
    }
    return (void*)NULL;
}

void* CFileBlock::thread_deal(void* arg) {
    while (!m_thread_deal.isQuit()) {
        if (m_thread_deal.isPause()) {
            m_thread_deal.wait();
        } else {
            // do something
        }
    }
    return (void*)NULL;
}

//            for (metadata = (struct fanotify_event_metadata *) events_buf;
//                    FAN_EVENT_OK(metadata, len);
//                    metadata = FAN_EVENT_NEXT(metadata, len)) {
//                fid = (struct fanotify_event_info_fid *) (metadata + 1);
//                file_handle = (struct file_handle *) fid->handle;

//                /* Ensure that the event info is of the correct type. */

//                if (fid->hdr.info_type == FAN_EVENT_INFO_TYPE_FID ||
//                    fid->hdr.info_type == FAN_EVENT_INFO_TYPE_DFID) {
//                    file_name = NULL;
//                } else if (fid->hdr.info_type == FAN_EVENT_INFO_TYPE_DFID_NAME) {
//                    file_name = file_handle->f_handle +
//                                file_handle->handle_bytes;
//                } else {
//                    fprintf(stderr, "Received unexpected event info type.\n");
//                    exit(EXIT_FAILURE);
//                }

//                if (metadata->mask == FAN_CREATE)
//                    printf("FAN_CREATE (file created):\n");

//                if (metadata->mask == (FAN_CREATE | FAN_ONDIR))
//                    printf("FAN_CREATE | FAN_ONDIR (subdirectory created):\n");

//             /* metadata->fd is set to FAN_NOFD when the group identifies
//                objects by file handles.  To obtain a file descriptor for
//                the file object corresponding to an event you can use the
//                struct file_handle that's provided within the
//                fanotify_event_info_fid in conjunction with the
//                open_by_handle_at(2) system call.  A check for ESTALE is
//                done to accommodate for the situation where the file handle
//                for the object was deleted prior to this system call. */

//                event_fd = open_by_handle_at(mount_fd, file_handle, O_RDONLY);
//                if (event_fd == -1) {
//                    if (errno == ESTALE) {
//                        printf("File handle is no longer valid. "
//                                "File has been deleted\n");
//                        continue;
//                    } else {
//                        perror("open_by_handle_at");
//                        exit(EXIT_FAILURE);
//                    }
//                }

//                snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d",
//                        event_fd);

//                /* Retrieve and print the path of the modified dentry. */

//                path_len = readlink(procfd_path, path, sizeof(path) - 1);
//                if (path_len == -1) {
//                    perror("readlink");
//                    exit(EXIT_FAILURE);
//                }

//                path[path_len] = '\0';
//                printf("\tDirectory '%s' has been modified.\n", path);

//                if (file_name) {
//                    ret = fstatat(event_fd, file_name, &sb, 0);
//                    if (ret == -1) {
//                        if (errno != ENOENT) {
//                            perror("fstatat");
//                            exit(EXIT_FAILURE);
//                        }
//                        printf("\tEntry '%s' does not exist.\n", file_name);
//                    } else if ((sb.st_mode & S_IFMT) == S_IFDIR) {
//                        printf("\tEntry '%s' is a subdirectory.\n", file_name);
//                    } else {
//                        printf("\tEntry '%s' is not a subdirectory.\n",
//                                file_name);
//                    }
//                }

//                /* Close associated file descriptor for this event. */

//                close(event_fd);
//            }

//            printf("All events processed successfully. Program exiting.\n");
//            exit(EXIT_SUCCESS);
//        }