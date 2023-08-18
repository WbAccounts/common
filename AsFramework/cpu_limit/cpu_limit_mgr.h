#ifndef CPU_LIMIT_CPU_LIMIT_MGR_H
#define CPU_LIMIT_CPU_LIMIT_MGR_H

#include <pthread.h>
#include <set>
#include "qh_thread/locker.hpp"
#include "common/singleton.hpp"

class CpuLimitManager {
  public:
    CpuLimitManager();
    ~CpuLimitManager();

  public:
    bool Init();
    void SyncStop();

    void SetCpuLimitMode(int mode);   // 0 -> 100%, 1 -> 25%, 2 -> 50%
    void SetCpuLimitSpeed(int speed); // 1% ~ 100%
    void AddThread(pthread_t thread);
    void DeleteThread(pthread_t thread);

    void SpeedCtrl();
    bool IsOk() { return m_init; }

  private:
    void SetCpuLimit();

  private:
    volatile bool m_init;
    volatile unsigned int m_speed;
    QH_THREAD::CMutex m_lock;
    std::set<pthread_t> m_threadset;
};

#endif /* CPU_LIMIT_CPU_LIMIT_MGR_H */
 
