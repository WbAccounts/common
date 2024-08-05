#ifndef M_COMMON_THREAD_LOCKER_HPP
#define M_COMMON_THREAD_LOCKER_HPP

#include "locker_base.hpp"

#define auto_locker   locker
#define manual_locker locker

namespace auto_locker {
class CAutoMutexLocker {
public:
    CAutoMutexLocker(locker_base::CMutexLock *locker) : m_mutex(locker), nError(0) { 
        nError = pthread_mutex_lock(m_mutex->getMutex()); 
    }
    ~CAutoMutexLocker() { 
        nError = pthread_mutex_unlock(m_mutex->getMutex()); 
    }
public:
    int getError() { return nError; }

private:
    int nError;
    locker_base::CMutexLock *m_mutex;
};

class CAutoRLocker {
public:
    CAutoRLocker(locker_base::CRwLock *locker) : m_rwlock(locker), nError(0) { 
        nError = pthread_rwlock_rdlock(m_rwlock->getRwlock()); 
    }
    ~CAutoRLocker() { 
        nError = pthread_rwlock_unlock(m_rwlock->getRwlock()); 
    }
public:
    int getError() { return nError; }

private:
    int nError;
    locker_base::CRwLock *m_rwlock;
};

class CAutoWLocker {
public:
    CAutoWLocker(locker_base::CRwLock *locker) : m_rwlock(locker), nError(0) { 
        nError = pthread_rwlock_wrlock(m_rwlock->getRwlock()); 
    }
    ~CAutoWLocker() { 
        nError = pthread_rwlock_unlock(m_rwlock->getRwlock()); 
    }
public:
    int getError() { return nError; }

private:
    int nError;
    locker_base::CRwLock *m_rwlock;
};
}

namespace manual_locker {
class CManualMutexLocker {
public:
    CManualMutexLocker(locker_base::CMutexLock *locker) : m_mutex(locker) { }
    ~CManualMutexLocker() {}
public:
    int lock() { return pthread_mutex_lock(m_mutex->getMutex()); }
    int unlock() { return pthread_mutex_unlock(m_mutex->getMutex()); }
private:
    locker_base::CMutexLock *m_mutex;
};

class CManualRLocker {
public:
    CManualRLocker(locker_base::CRwLock *locker) : m_rwlock(locker) { }
    ~CManualRLocker() {}
public:
    int lock() { return pthread_rwlock_rdlock(m_rwlock->getRwlock()); }
    int unlock() { return pthread_rwlock_unlock(m_rwlock->getRwlock()); }
private:
    locker_base::CRwLock *m_rwlock;
};

class CManualWLocker {
public:
    CManualWLocker(locker_base::CRwLock *locker) : m_rwlock(locker) { }
    ~CManualWLocker() {}
public:
    int lock() { return pthread_rwlock_wrlock(m_rwlock->getRwlock()); }
    int unlock() { return pthread_rwlock_unlock(m_rwlock->getRwlock()); }
private:
    locker_base::CRwLock *m_rwlock;
};
}

#endif