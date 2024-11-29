#ifndef M_COMMON_THREAD_HPP
#define M_COMMON_THREAD_HPP

#include "locker.hpp"
#include "cond.hpp"
#include <tr1/functional>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

namespace thread {
typedef std::tr1::function<void(void*)> ThreadFunc;
class CThread {
public:
    CThread(ThreadFunc func, void *arg = NULL) : m_thread(0), m_func(func), m_arg(arg), m_isPause(false), m_isQuit(false) {
        m_cond = new conder::CConder(&m_mutex);
    }
    virtual ~CThread() {
        if (isRunning()) {
            usleep(1000 * 0.1);
            quit();
            join();
        }
        if (m_cond) delete m_cond;
    }

public:
    void run();
    void join();
    void tryJoin();
    void detach();
    void wait(int sec = 0, int nsec = 0);

    void pause();
    void quit();

    bool isPause();
    bool isQuit();

    bool isRunning();
    bool isRealQuit();

    void broad();
    void signal();

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

void CThread::run() {
    locker::CAutoMutexLocker locker(&m_mutex);
    pthread_create(&m_thread, NULL, &CThread::thread_func, this);
}

void CThread::join() {
    locker::CAutoMutexLocker locker(&m_mutex);
    pthread_join(m_thread, NULL);
}

void CThread::tryJoin() {
    locker::CAutoMutexLocker locker(&m_mutex);
    pthread_tryjoin_np(m_thread, NULL);
}

void CThread::detach() {
    locker::CAutoMutexLocker locker(&m_mutex);
    pthread_detach(m_thread);
}

void CThread::wait(int sec, int nsec) {
    locker::CManualMutexLocker locker(&m_mutex);
    locker.lock();
    switch (sec) {
        case  0: m_cond->Wait(); break;
        default: m_cond->Wait(sec, nsec); break;
    }
    locker.unlock();
}

void CThread::pause() {
    locker::CAutoMutexLocker locker(&m_mutex);
    m_isPause = true;
}

void CThread::quit() {
    locker::CAutoMutexLocker locker(&m_mutex);
    m_isQuit = true;
    m_cond->BroadCast();
}

bool CThread::isRunning() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return pthread_kill(m_thread, 0) == 0;
}

bool CThread::isRealQuit() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return pthread_kill(m_thread, 0) == ESRCH;
}

bool CThread::isPause() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return m_isPause;
}

bool CThread::isQuit() {
    locker::CAutoMutexLocker locker(&m_mutex);
    return m_isQuit;
}

void CThread::broad() {
    locker::CAutoMutexLocker locker(&m_mutex);
    m_isPause = false;
    m_cond->BroadCast();
}

void CThread::signal() {
    locker::CAutoMutexLocker locker(&m_mutex);
    m_isPause = false;
    m_cond->Signal();
}

bool CThread::set_thread_name(const char* thread_name) {
    return (pthread_setname_np(m_thread, thread_name) == 0);
}

}   // namespace thread

#endif