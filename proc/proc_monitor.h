#ifndef PROC_MONITOR_H
#define PROC_MONITOR_H

#include "thread/thread_utils/thread.hpp"
#include <tr1/functional>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <signal.h>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <vector>


using CallBack = std::tr1::function<void(const struct ProcDealInfo&)>;

struct ProcSubInfo {
    std::string module;
    std::map<int, std::vector<std::string> > flow_map;
    std::map<int, CallBack> event_call;
};

struct ProcInfo {
    int pid;
    int tgid;
    std::string proc_exe;
    std::string proc_cwd;
    std::string proc_cmd;
    std::string proc_name;
    ProcInfo() : pid(0), tgid(0) {}
};

struct ProcDealInfo {
    ProcInfo info;
    int event_type;
    ProcDealInfo() : event_type(0) {}
};

enum PROC_ERROR {
    proc_error_ok           = 0B00000001,
    change_proc_on_error    = 0B00000010,
    change_proc_off_error   = 0B00000100,
    un_subscribe_error      = 0B00001000,
    un_add_flow_error       = 0B00010000,
};

class CProcMonitor {
public:
    CProcMonitor();
    ~CProcMonitor();

    bool init();

    PROC_ERROR sub_proc(ProcSubInfo& sub_info);
    PROC_ERROR un_sub_proc(ProcSubInfo& sub_info);

    PROC_ERROR get_sys_proc(std::map<int, ProcInfo> &sys_proc);

private:
    void* thread_function_monitor(void* parm);
    void* thread_function_deal(void* parm);

    void deal_event(const ProcDealInfo& info);
    bool change_cn_proc_mode(int mode);

    bool AddDef();
    bool DelDef();

    bool get_sys_proc();
    bool get_proc_info(const std::string &pid, ProcInfo &info);
    bool is_monitor_proc(const int &type, const std::string &proc);

public:
    static locker_base::CMutexLock m_mutex_init;
    static CProcMonitor *m_ProcMonitor;

    static CProcMonitor* get_instance() {
        if (m_ProcMonitor == NULL) {
            locker::CAutoMutexLocker locker(&m_mutex_init);
            if (m_ProcMonitor == NULL) {
                m_ProcMonitor = new CProcMonitor();
                if (!m_ProcMonitor->init()) {
                    delete m_ProcMonitor;
                    m_ProcMonitor = NULL;
                }
            }
        }
        return m_ProcMonitor;
    }

private:
    volatile int m_def;
    volatile int m_socket_fd;
    
    locker_base::CMutexLock m_mutex_sub;
    locker_base::CMutexLock m_mutex_deal;

    thread::CThread m_thread_monitor;
    thread::CThread m_thread_deal;

    struct sockaddr_nl m_daddr;
    struct sockaddr_nl m_l_local;

    std::map<const std::string, ProcSubInfo> m_sub_map; // 维护一个订阅表
    std::queue<ProcDealInfo> m_deal_queue;  // 维护一个事件待处理队列
    std::map<int, ProcInfo>  m_sys_proc;    // 维护一个系统的进程状态表
};

#define PROC_MONITOR_INSTANCE   CProcMonitor::get_instance()

#endif // PROC_MONITOR_H