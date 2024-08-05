#ifndef M_COMMON_THREAD_HPP
#define M_COMMON_THREAD_HPP

#include "locker.hpp"
#include "cond.hpp"
#include <tr1/functional>

namespace thread {

typedef std::tr1::function<void(void*)> ThreadFunc;

static void* thread_func(void* arg) {

    ThreadFunc* func = (ThreadFunc*)arg;
    (*func);
    return NULL;
}

template <typename Tfunc, typename Targ>
class CThread {
public:
    CThread(Targ *arg) : m_pArg(arg) {
        pthread_create(&m_thread, NULL, &CThread::thread_func, arg);
    }
    virtual ~CThread() {}

private:
    void *thread_func(void *arg) {
        Targ *Arg = (Targ*)arg;
        if (m_func) {
            m_func(Arg);
        }
        return NULL;
    }

private:
    int m_nError;
    pthread_t m_thread;
    Targ  *m_pArg;
    Tfunc *m_func;
};
}

#endif