#include "proc_monitor.h"
#include "proc_utils.hpp"
#include "log/log_helper.h"

#include <dirent.h>
#include <regex>

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#define _LINUX_TIME_H
#include <unistd.h>

#define MAX_MSGSIZE 256
#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif

volatile static int num_event = 0;
volatile static int num_deal = 0;

locker_base::CMutexLock CProcMonitor::m_mutex_init;
CProcMonitor *CProcMonitor::m_ProcMonitor = NULL;   
CProcMonitor::CProcMonitor() {
    m_def = 0;
    m_socket_fd = 0;
    memset(&m_daddr, 0, sizeof(struct sockaddr_nl));
}

CProcMonitor::~CProcMonitor() {
    change_cn_proc_mode(PROC_CN_MCAST_IGNORE);
    close(m_socket_fd);
}

bool CProcMonitor::init() {
    m_daddr.nl_family = AF_NETLINK;
    m_daddr.nl_pid = 0;
    m_daddr.nl_groups = CN_IDX_PROC;

    m_socket_fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);

    memset(&m_l_local, 0, sizeof(struct sockaddr_nl));
    m_l_local.nl_family = AF_NETLINK;
    m_l_local.nl_groups = CN_IDX_PROC;
    m_l_local.nl_pid = getpid();

    if (bind(m_socket_fd, (struct sockaddr *)&m_l_local, sizeof(struct sockaddr_nl)) == -1) {
        LOG_ERROR("CProcMonitor init bind error");
        close(m_socket_fd);
        return false;
    }

    m_thread_monitor.set_thread_func(std::tr1::bind(&CProcMonitor::thread_function_monitor, this, std::tr1::placeholders::_1));
    m_thread_monitor.run();
    m_thread_monitor.pause();

    m_thread_deal.set_thread_func(std::tr1::bind(&CProcMonitor::thread_function_deal, this, std::tr1::placeholders::_1));
    m_thread_deal.run();
    return true;
}

PROC_ERROR CProcMonitor::sub_proc(ProcSubInfo& sub_info) {
    locker::CAutoMutexLocker locker(&m_mutex_sub);
    m_sub_map[sub_info.module] = sub_info;

    if (!AddDef())
        return change_proc_on_error;
    return proc_error_ok;
}

PROC_ERROR CProcMonitor::un_sub_proc(ProcSubInfo& sub_info) {
    using Map = std::map<const std::string, ProcSubInfo>;

    locker::CAutoMutexLocker locker(&m_mutex_sub);
    Map::iterator it = m_sub_map.find(sub_info.module);
    if (it != m_sub_map.end())
        m_sub_map.erase(it++);

    if (!DelDef())
        return change_proc_off_error;
    return proc_error_ok;
}

bool CProcMonitor::AddDef() { 
    if (++m_def > 0 && m_thread_monitor.isPause()) {
        if (!change_cn_proc_mode(PROC_CN_MCAST_LISTEN)) {
            return false;
        }
        if (!get_sys_proc()) {
            change_cn_proc_mode(PROC_CN_MCAST_IGNORE);
            return false;
        }
        m_thread_monitor.broad(); 
    }
    return true;
}

bool CProcMonitor::DelDef() { 
    if (--m_def == 0 && !m_thread_monitor.isPause()) {
        if (change_cn_proc_mode(PROC_CN_MCAST_IGNORE)) {
            return false;
        }
        m_thread_monitor.pause();
    }
    return true;
}

void* CProcMonitor::thread_function_monitor(void* parm) {
    while (!m_thread_monitor.isQuit()) { 
        if (m_thread_monitor.isPause()) {
            m_thread_monitor.wait();
        } else {
            struct nlmsghdr nlhdr;
            struct iovec    iov;
            struct msghdr   msg;

            memset(&nlhdr, 0, sizeof(NLMSG_SPACE(MAX_MSGSIZE)));
            memset(&iov, 0, sizeof(struct iovec));
            memset(&msg, 0, sizeof(struct msghdr));

            iov.iov_base = (void *)&nlhdr;
            iov.iov_len = NLMSG_SPACE(MAX_MSGSIZE);
            msg.msg_name = (void *)&m_daddr;
            msg.msg_namelen = sizeof(m_daddr);
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;

            int ret = recvmsg(m_socket_fd, &msg, 0);
            if (ret <= 0) {
                LOG_DEBUG("proc monitor recvmsg error[{}]", ret);
                continue;
            } else {
                struct cn_msg *cnmsg = (struct cn_msg *)NLMSG_DATA(&nlhdr);
                struct proc_event procevent = *(struct proc_event *)cnmsg->data;

                // 获取相关信息
                ProcDealInfo deal_info;
                if (procevent.what == proc_event::PROC_EVENT_EXEC) 
                {
                    if (!get_proc_info(std::to_string(procevent.event_data.exec.process_pid), deal_info.info)) {
                        continue;
                    }
                    m_sys_proc[procevent.event_data.exec.process_pid] = deal_info.info;
                    if (!is_monitor_proc(procevent.what, deal_info.info.proc_name))
                        continue;

                    deal_info.event_type = proc_event::PROC_EVENT_EXEC;
                    LOG_DEBUG("recv exec event, process name[{}] pid[{}]", deal_info.info.proc_name.c_str(), deal_info.info.pid);
                } 
                else if (procevent.what == proc_event::PROC_EVENT_EXIT) 
                {
                    if (m_sys_proc.find(procevent.event_data.exec.process_pid) != m_sys_proc.end()) {
                        deal_info.info.pid = procevent.event_data.exec.process_pid;
                    } else if (m_sys_proc.find(procevent.event_data.exec.process_tgid) != m_sys_proc.end()) {
                        deal_info.info.pid = procevent.event_data.exec.process_tgid;
                    } else {
                        continue;
                    }
                    deal_info.info = m_sys_proc[deal_info.info.pid];
                    deal_info.event_type = procevent.what;
                    m_sys_proc.erase(deal_info.info.pid);

                    if (!is_monitor_proc(procevent.what, deal_info.info.proc_name))
                        continue;

                    LOG_DEBUG("recv exit event, process name[{}] pid[{}]", deal_info.info.proc_name.c_str(), deal_info.info.pid);
                } else {
                    continue;
                }
                num_event++;
                // 处理 
                locker::CAutoMutexLocker locker(&m_mutex_deal);
                m_deal_queue.push(deal_info);
                if (m_thread_deal.isPause()) m_thread_deal.broad();
            }
        }
    }
    return (void*)NULL;
}

void* CProcMonitor::thread_function_deal(void* parm) {
    while (!m_thread_deal.isQuit()) {
        std::queue<ProcDealInfo> Tmp;
        {
            locker::CAutoMutexLocker locker(&m_mutex_deal);
            m_deal_queue.swap(Tmp);
        }

        if (Tmp.empty()) {
            m_thread_deal.pause();
            m_thread_deal.wait();
        } else {
            while (!Tmp.empty()) {
                num_deal++;
                ProcDealInfo info = Tmp.front();
                Tmp.pop();
                LOG_DEBUG("start deal recved event, process name[{}]  pid[{}]  event[{}]", info.info.proc_name.c_str(),
                                                                                      info.info.pid, info.event_type);
                deal_event(info);
            }
            LOG_DEBUG("process monitor has dealed [{}]  recved[{}]", num_deal, num_event);
        }
    }
    return (void*)NULL;
}

void CProcMonitor::deal_event(const ProcDealInfo& info) {
    locker::CAutoMutexLocker locker(&m_mutex_sub);
    std::map<const std::string, ProcSubInfo>::iterator it = m_sub_map.begin();
    for (; it != m_sub_map.end(); ++it) {
        ProcSubInfo &sub = it->second;
        if ((sub.flow_map).find(info.event_type) != (sub.flow_map).end()) {
            std::vector<std::string> &vec = sub.flow_map.find(info.event_type)->second;
            for (int index=0; index<vec.size(); ++index) {
                if (vec[index] == info.info.proc_name) {
                    sub.event_call.find(info.event_type)->second(info);
                    break;
                }
            }
        }
    }
}

bool CProcMonitor::change_cn_proc_mode(int mode)
{
    struct nlmsghdr nlhdr;
    struct iovec    iov;
    struct msghdr   msg;

    memset(&nlhdr, 0, sizeof(NLMSG_SPACE(MAX_MSGSIZE)));
    memset(&iov, 0, sizeof(struct iovec));
    memset(&msg, 0, sizeof(struct msghdr));

    struct cn_msg *cnmsg = (struct cn_msg *)NLMSG_DATA(&nlhdr);
    int *connector_mode = (int *)cnmsg->data;
    *connector_mode = mode;

    nlhdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op));
    nlhdr.nlmsg_pid = getpid();
    nlhdr.nlmsg_flags = 0;
    nlhdr.nlmsg_type = NLMSG_DONE;
    nlhdr.nlmsg_seq = 0;

    cnmsg->id.idx = CN_IDX_PROC;
    cnmsg->id.val = CN_VAL_PROC;
    cnmsg->seq = 0;
    cnmsg->ack = 0;
    cnmsg->len = sizeof(enum proc_cn_mcast_op);

    iov.iov_base = (void *)&nlhdr;
    iov.iov_len = nlhdr.nlmsg_len;
    msg.msg_name = (void *)&m_daddr;
    msg.msg_namelen = sizeof(m_daddr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    int ret = sendmsg(m_socket_fd, &msg, 0);
    if (ret == -1) {
        LOG_ERROR("change_cn_proc_mode sendmsg error[{}]", ret);
        return false;
    }
    return true;
}

bool CProcMonitor::get_sys_proc() {
    DIR *dirp;
    if((dirp = opendir("/proc")) == NULL) {
        LOG_ERROR("open dir [/proc] failed, get running process failed");
        return false;
    }
    struct dirent *entry;
    while((entry = readdir(dirp)) != NULL) {
        if(S_ISDIR(entry->d_type)) {
            if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0) {
                continue;
            }
            ProcInfo info;
            if (!get_proc_info(entry->d_name, info)) {
                continue;
            }
            int pid = std::stoi(entry->d_name);
            m_sys_proc[pid] = info;
        }
    }
    closedir(dirp);
    return true;
}

bool CProcMonitor::get_proc_info(const std::string &pid, ProcInfo &info) {
    char status_path[64];
    snprintf(status_path, 64, "/proc/%s/status", pid.c_str());

    if (file_utils::is_exist(status_path) && file_utils::is_file(status_path))
    {
        std::string data;
        try {
            std::string line;
            std::ifstream ifs(status_path);
            std::regex reg_name("^Name:[ \\s]*(.*)[ \\s]*$");
            std::regex reg_pid("^Pid:[ \\s]*(\\d+)[ \\s]*$");
            std::regex reg_tgid("^Tgid:[ \\s]*(\\d+)[ \\s]*$");
            std::smatch mac_name, mac_pid, mac_tgid;
            while (getline(ifs, line)) {
                if (mac_name.empty()) {
                    if (std::regex_match(line, mac_name, reg_name)) {
                        info.proc_name = mac_name[1];
                        continue;
                    }
                } 
                else if (mac_pid.empty()) {
                    if (std::regex_match(line, mac_pid, reg_pid)) {
                        info.pid = std::stoi(mac_pid[1]);
                        continue;
                    }
                } 
                else if (mac_tgid.empty()) {
                    if (std::regex_match(line, mac_tgid, reg_tgid)) {
                        info.tgid = std::stoi(mac_tgid[1]);
                        continue;;
                    }
                }
            }
            info.proc_cmd = proc_utils::get_cmd_line(std::stoi(pid));
            info.proc_cwd = proc_utils::get_cwd_name(std::stoi(pid));
            info.proc_exe = proc_utils::get_exe_name(std::stoi(pid));

        } catch (...) {
            LOG_ERROR("Finding a error during reading file of [{}]", status_path);
            return false;
        }
    } else {
        LOG_ERROR("process [{}] does not have file [status]", pid.c_str());
        return false;
    }
    return true;
}

bool CProcMonitor::is_monitor_proc(const int &type, const std::string &proc) {
    std::map<const std::string, ProcSubInfo>::iterator it = m_sub_map.begin();
    for (; it != m_sub_map.end(); ++it) {
        std::map<int, std::vector<std::string> >::iterator it_flow = it->second.flow_map.find(type);
        if (it_flow != it->second.flow_map.end()) {
            for (int index=0; index<it_flow->second.size(); ++index) {
                if (it_flow->second[index] == proc) {
                    return true;
                }
            }
        }
    }
    return false;
}

PROC_ERROR CProcMonitor::get_sys_proc(std::map<int, ProcInfo> &sys_proc) {
    sys_proc = m_sys_proc;
    return proc_error_ok;
}