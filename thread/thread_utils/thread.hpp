#ifndef M_COMMON_THREAD_HPP
#define M_COMMON_THREAD_HPP

#include "locker.hpp"
#include "cond.hpp"
#include <tr1/functional>
#include <signal.h>
#include <errno.h>

namespace thread {
typedef std::tr1::function<void(void*)> ThreadFunc;
class CThread {
public:
    CThread(ThreadFunc func, void *arg = NULL) : m_thread(0), m_func(func), m_arg(arg), m_isPause(false), m_isQuit(false) {
        m_cond = new conder::CConder(&m_mutex);
    }
    virtual ~CThread() {
        if (isRunning()) {
            quit();
            join();
        }
        if (m_cond) delete m_cond;
    }

public:
    bool run();
    bool join();
    bool tryJoin();
    bool detach();
    bool wait(int sec = 0, int nsec = 0);

    void pause();
    bool quit();

    bool isPause();
    bool isQuit();

    bool isRunning();
    bool isRealQuit();

    bool broad();
    bool signal();

    bool set_thread_name(const char* thread_name);
    pthread_t getThreadId() { return m_thread; }

private:
    static void *thread_func(void *cl) {
        CThread *thread = (CThread*)cl;
        thread->m_func(thread->m_arg);
        return NULL;
    }

private:
    void *m_arg;
    ThreadFunc m_func;
    pthread_t m_thread;
    conder::CConder *m_cond;
    locker_base::CMutexLock m_mutex;
    locker_base::CMutexLock m_condMutex;
    bool m_isPause;
    bool m_isQuit;
};

bool CThread::run() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return (pthread_create(&m_thread, NULL, &CThread::thread_func, this) == 0);
}

bool CThread::join() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return (pthread_join(m_thread, NULL) == 0);
}

bool CThread::tryJoin() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return (pthread_tryjoin_np(m_thread, NULL) == 0);
}

bool CThread::detach() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return (pthread_detach(m_thread) == 0);
}

bool CThread::wait(int sec, int nsec) {
    locker::CAutoMutexLocker locker(&m_mutex);
    if (sec == 0) {
        return (m_cond->Wait() == 0);
    }
    return (m_cond->Wait(sec, nsec) == 0);
}

void CThread::pause() {
    locker::CAutoMutexLocker locker(&m_mutex);
    m_isPause = true;
}

bool CThread::quit() {
    locker::CAutoMutexLocker locker(&m_mutex);
    m_isQuit = true;
    return (m_cond->BroadCast() == 0);
}

bool CThread::isRunning() {
    locker::CAutoMutexLocker locker(&m_mutex);
    if (m_thread)
        return (pthread_kill(m_thread, 0) == 0);
    return false;
}

bool CThread::isRealQuit() {
    locker::CAutoMutexLocker locker(&m_mutex);
    if (m_thread)
        return (pthread_kill(m_thread, 0) == ESRCH);
    return true;
}

bool CThread::isPause() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return m_isPause;
}

bool CThread::isQuit() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return m_isQuit;
}

bool CThread::broad() {
    locker::CAutoMutexLocker locker(&m_mutex);
    m_isPause = false;
    return (m_cond->BroadCast() == 0);
}

bool CThread::signal() {
    locker::CAutoMutexLocker locker(&m_mutex);
    m_isPause = false;
    return (m_cond->Signal() == 0);
}

bool CThread::set_thread_name(const char* thread_name) {
    return (pthread_setname_np(m_thread, thread_name) == 0);
}

}   // namespace thread

#endif