#ifndef M_COMMON_THREAD_COND_HPP
#define M_COMMON_THREAD_COND_HPP

#include "locker_base.hpp"

namespace conder {
class CConder {
public:
    CConder(locker_base::CMutexLock *mutex = NULL) {
        if (mutex) Init(mutex);
    }

    ~CConder() {
        pthread_condattr_destroy(&m_condattr);
        pthread_cond_destroy(&m_cond);
    }
public:
    int Init(locker_base::CMutexLock* mutex) {
        m_mutex = mutex;
        pthread_condattr_init(&m_condattr);
        pthread_condattr_setclock(&m_condattr, CLOCK_MONOTONIC);
        return pthread_cond_init(&m_cond, &m_condattr);
    }
    
    int Signal() { return pthread_cond_signal(&m_cond); }
    int BroadCast() { return pthread_cond_broadcast(&m_cond); }
    int Wait() { return pthread_cond_wait(&m_cond, m_mutex->getMutex()); }
    int Wait(int sec, int nsec = 0) { 
        struct timespec timer_t;
        timer_t.tv_sec = sec;
        timer_t.tv_nsec = nsec;
        return pthread_cond_timedwait (&m_cond, m_mutex->getMutex(), &timer_t); 
    }
private:
    locker_base::CMutexLock *m_mutex;
    pthread_condattr_t m_condattr;
    pthread_cond_t m_cond;
};
}

#endif