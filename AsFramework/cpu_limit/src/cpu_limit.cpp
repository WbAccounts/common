#include "cpu_limit/include/cpu_limit.h"
#include <time.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/time.h>
#include "common/log/log.hpp"

#define ALFA 0.08

#ifndef HZ
# if defined(_SC_CLK_TCK) \
     && (!defined(OPENSSL_SYS_VMS) || __CTRL_VER >= 70000000)
#  define HZ ((double)sysconf(_SC_CLK_TCK))
# else
#  ifndef CLK_TCK
#   ifndef _BSD_CLK_TCK_ /* FreeBSD hack */
#    define HZ  100.0
#   else /* _BSD_CLK_TCK_ */
#    define HZ ((double)_BSD_CLK_TCK_)
#   endif
#  else /* CLK_TCK */
#   define HZ ((double)CLK_TCK)
#  endif
# endif
#endif

CPULimit::CPULimit()
    : m_nSpeed(100)
    , m_dCpuUsage(-0.001) {
    init();
}

CPULimit::~CPULimit() {
    uninit();
}

void CPULimit::init() {
    m_nCPUNums = getCPUNum();
    if (m_nCPUNums <= 0) {
        m_nCPUNums = 1;
    }
    m_pid = ::getpid();

    struct timeval t1;
    gettimeofday(&t1, NULL);
    int jiffes1 = getJiffies();
    usleep(100 * 1000);
    struct timeval t2;
    gettimeofday(&t2, NULL);
    int jiffes2 = getJiffies();
    long dt = timeDiff(&t2, &t1);
    m_dCpuUsage = ((double)(jiffes1 - jiffes2)) / (dt * HZ / 1000000.0);

    m_tLastSample = t2;
    m_nLastJiffies = jiffes2;

    if (checkProc()) {
        QH_THREAD::CThread::run(NULL);
    }
}

void CPULimit::uninit() {
    QH_THREAD::CThread::quit();
    QH_THREAD::CThread::join();
}

int CPULimit::getCPUNum() {
    int iNumCpu = -1;
#ifdef _SC_NPROCESSORS_ONLN
    iNumCpu = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
    int mib[2] = {CTL_HW, HW_NCPU};
    size_t len = sizeof(iNumCpu);
    sysctl(mib, 2, &iNumCpu, &len, NULL, 0);
#endif
    return iNumCpu;
}

int CPULimit::monitor() {
    double cur_speed = m_dCpuUsage * 100.0 / m_nCPUNums;
    if ((double)m_nSpeed < cur_speed) {
        LOG_DEBUG(">>>>>>>>>>>>>>>>>> supper speed >>>>>>>>>>>>>>>>>>>>>>");
        LOG_DEBUG("limit speed %lf, curent speed %lf.", (double) m_nSpeed, (double) cur_speed);
        usleep(100 * 1000);
    }

    return 1;
}

int CPULimit::checkProc() {
    struct statfs mnt;
    if (statfs("/proc", &mnt) < 0) {
        LOG_ERROR("system no /proc directory");
        return 0;
    }

    if (mnt.f_type != 0x9fa0) {
        return 0;
    }

    return 1;
}

int CPULimit::setSpeed(unsigned int iSpeed, pthread_t thread) {
    m_nSpeed = iSpeed;
    if (m_nCPUNums == 1) {
        LOG_INFO("set thread %ld, cpu speed mode[%d].", thread, iSpeed);
        return 0;
    }
    int processorNums = (m_nCPUNums * iSpeed) / 100;
    if (processorNums == 0) {
        processorNums = 1;
    }

    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < processorNums; i++) {
        CPU_SET(i, &mask);
    }

    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &mask) != 0) {
        LOG_ERROR("set cpu affinity failed, because:%s[%d].", strerror(errno), errno);
        return -1;
    } else {
        LOG_DEBUG("set thread %ld, cpu speed mode[%d], success to bind %d cpus.", thread, m_nSpeed, processorNums);
    }

    return 0;
}

int CPULimit::getJiffies() {
    char file[20] = {0};
    snprintf(file, sizeof(file), "/proc/%d/stat", m_pid);
    FILE* fd = fopen(file, "r");
    if (fd == NULL) {
        LOG_ERROR("open file[%s] for get time failed, because:%s[%d].", file, strerror(errno), errno);
        return -1;
    }
    char buffer[1024] = {0};
    if (fgets(buffer, sizeof(buffer), fd) == NULL) {
        LOG_ERROR("get content for get time failed, because:%s[%d].", strerror(errno), errno);
        fclose(fd);
        return -1;
    }
    fclose(fd);

    char* ptr = buffer;
    char* pend = buffer + sizeof(buffer) - 1;
    ptr = (char*)memchr(ptr + 1, ')',pend - ptr);
    int sp = 12;
    while (sp--) {
        ptr = (char*)memchr(ptr + 1, ' ',pend - ptr);
    }
    int utime = atoi(ptr + 1);
    ptr = (char*)memchr(ptr + 1, ' ',pend - ptr);
    int ktime = atoi(ptr + 1);
    return utime + ktime;
}

unsigned long CPULimit::timeDiff(const struct timeval* t1, const struct timeval* t2) {
    return (t1->tv_sec - t2->tv_sec) * 1000000 + (t1->tv_usec - t2->tv_usec);
}

int CPULimit::calcateProcessCpuUsage() {
    int jiffes = getJiffies();
    if (jiffes < 0) {
        return -1;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    long dt = timeDiff(&now, &m_tLastSample);

    double dMaxJiffies = dt * HZ / 1000000.0;
    double dSample = ((double)(jiffes - m_nLastJiffies)) / dMaxJiffies;
    m_dCpuUsage = (1 - ALFA) * m_dCpuUsage + ALFA * dSample;

    m_tLastSample = now;
    m_nLastJiffies = jiffes;

    return 0;
}

void* CPULimit::thread_function(void* param) {
    LOG_INFO("cpu speed control thread start.");
    while (!QH_THREAD::CThread::isQuit()) {
        calcateProcessCpuUsage();
        usleep(100 * 1000);
    }
    LOG_INFO("cpu speed control thread exit.");
    return NULL;
}
 
