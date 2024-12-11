#ifndef SINGLETON_CLASS_H
#define SINGLETON_CLASS_H

#include "thread/thread_utils/locker.hpp"

template <class T, typename... Args>
class CSingletonTemplate {
public:
    CSingletonTemplate() {}
    ~CSingletonTemplate() {}

public:
    static T* getInstance(Args... arg) {
        if (m_instance == NULL) {
            locker::CAutoMutexLocker locker(&m_mutex);
            if (m_instance == NULL) {
                m_instance = new T(arg...);
            }
        }
        return m_instance;
    }

    static void destroyInstance() {
        if (m_instance != NULL) {
            locker::CAutoMutexLocker locker(&m_mutex);
            if (m_instance != NULL) {
                delete m_instance;
                m_instance = NULL;
            }
        }
    }

private:
    static T* m_instance;
    static locker_base::CMutexLock m_mutex;
};

template <class T, typename... Args>
T* CSingletonTemplate<T, Args...>::m_instance = NULL;

template <class T, typename... Args>
locker_base::CMutexLock CSingletonTemplate<T, Args...>::m_mutex;

#endif // SINGLETON_CLASS_H