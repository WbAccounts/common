#ifndef FILE_BLOCK_H
#define FILE_BLOCK_H

#include "thread/thread_utils/thread.hpp"
#include <map>

using SubCallFunc = std::tr1::function<void(const std::string& path, int event)>;

using EV_OP = std::map<int, int>;   // <event, op>
using PA_EP = std::map<std::string, EV_OP>; // <path, <event, op>>


namespace file_block {
    struct SubInfo {
        PA_EP nodes;
        SubCallFunc func;
    };
};

class CFileBlock
{
public:
    CFileBlock();
    ~CFileBlock();
private:
    bool init();

private:
    void* thread_monitor(void* arg);
    void* thread_deal(void* arg);

public:
    // 一个id仅允许订阅一次，如果重复订阅，将会覆盖之前的订阅
    bool subscribe(const std::string& id, const file_block::SubInfo &sub);
    bool unsubscribe(const std::string& id);

    bool add(const std::string& id, const std::string& path, int event, int op);
    bool del(const std::string& id, const std::string& path, int event);

private:
    void update_ev_count(int event, bool isAdd);

private:
    int m_fd;

    thread::CThread m_thread_monitor;
    thread::CThread m_thread_deal;

    locker_base::CMutexLock m_sub_mutex;
    locker_base::CMutexLock m_deal_mutex;

    std::map<std::string, file_block::SubInfo> m_sub_map;  // <module id, subscribe>
    std::map<int, int>  m_ev_count; // <event, count>
};

#endif
