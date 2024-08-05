#ifndef M_COMMON_THREAD_LOCKER_BASE_HPP
#define M_COMMON_THREAD_LOCKER_BASE_HPP

#include <pthread.h>

// 使用未初始化的锁、销毁未初始化的锁会有未知风险，故做此封装
namespace locker_base {

class CMutexLock {
public:
    CMutexLock() { pthread_mutex_init(&m_mutex, NULL); }
    ~CMutexLock() { pthread_mutex_destroy(&m_mutex); }
public:
    pthread_mutex_t *getMutex() { return &m_mutex; }
private:
    pthread_mutex_t m_mutex;
};

class CRwLock {
public:
    CRwLock() { pthread_rwlock_init(&m_rwlock, NULL); }
    ~CRwLock() { pthread_rwlock_destroy(&m_rwlock); }
public:
    pthread_rwlock_t *getRwlock() { return &m_rwlock; }
private:
    pthread_rwlock_t m_rwlock;
};
};



#endif