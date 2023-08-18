#include "cpu_limit/cpu_limit_mgr.h"
#include "cpu_limit/include/cpu_limit.h"
#include "common/log/log.hpp"

CpuLimitManager::CpuLimitManager()
    : m_init(false)
    , m_speed(100) {
}

CpuLimitManager::~CpuLimitManager() {

}

bool CpuLimitManager::Init() {
    QH_THREAD::CMutexAutoLocker Lck(&m_lock);
    m_threadset.clear();
    m_init = true;
    return true;
}

void CpuLimitManager::SyncStop() {
    if (m_init) {
        m_init = false;
        m_speed = 100;
        Singleton<CPULimit>::Uninit();
        QH_THREAD::CMutexAutoLocker Lck(&m_lock);
        m_threadset.clear();
    }
}

void CpuLimitManager::SpeedCtrl() {
    if (m_init) {
        Singleton<CPULimit>::Instance().monitor();
    }
}

void CpuLimitManager::SetCpuLimitMode(int mode) {
    if (!m_init)
        return;

    if (mode < 0 || mode > 2) {
        LOG_ERROR("set cpu limit mode failed: unsupport mode %d.", mode);
        return;
    }

    unsigned int speed = 5 * mode;
    if (mode == 0) speed = 100;
    if (speed == m_speed) {
        LOG_INFO("set cpu limit mode, but unchanged speed %d.", speed);
        return;
    } else {
        m_speed = speed;
    }

    SetCpuLimit();
}

void CpuLimitManager::SetCpuLimitSpeed(int speed) {
    unsigned int uspeed = (unsigned int)speed;
    if (uspeed == m_speed) {
        LOG_INFO("set cpu limit mode, but unchanged speed %d.", speed);
        return;
    } else if (speed <= 0) {
        m_speed = 1;
    } else if (speed > 100) {
        m_speed = 100;
    } else {
        m_speed = uspeed;
    }

    SetCpuLimit();
}

void CpuLimitManager::SetCpuLimit() {
    std::set<pthread_t> threadset_copy;
    {
        QH_THREAD::CMutexAutoLocker Lck(&m_lock);
        if (!m_threadset.empty()) {
            threadset_copy.insert(m_threadset.begin(), m_threadset.end());
        }
    }

    std::set<pthread_t> threadset_failed;
    std::set<pthread_t>::iterator it = threadset_copy.begin();
    for (; it != threadset_copy.end(); ++it) {
        if (Singleton<CPULimit>::Instance().setSpeed(m_speed, *it) != 0) {
            threadset_failed.insert(*it);
        }
    }

    //移除 由于线程不存在，导致的绑定失败的 线程
    if (!threadset_failed.empty()) {
        it = threadset_failed.begin();
        for (; it != threadset_failed.end(); ++it) {
            DeleteThread(*it);
        }
    }
}

void CpuLimitManager::AddThread(pthread_t thread) {
    //线程不存在，导致绑定失败， 线程不加入队列中
    if (m_init && Singleton<CPULimit>::Instance().setSpeed(m_speed, thread) == 0) {
        LOG_DEBUG("add thread to cpulimit manager success");
        QH_THREAD::CMutexAutoLocker Lck(&m_lock);
        m_threadset.insert(thread);
    } else {
        LOG_ERROR("add thread to cpulimit manager failed");
    }
}

void CpuLimitManager::DeleteThread(pthread_t thread) {
    QH_THREAD::CMutexAutoLocker Lck(&m_lock);
    if (m_init && !m_threadset.empty()) {
        std::set<pthread_t>::iterator it = m_threadset.find(thread);
        if (it != m_threadset.end()) {
            LOG_DEBUG("delete thread from cpulimit manager");
            m_threadset.erase(it);
        }
    }
}
 
