#ifndef CPU_LIMIT_CPULIMIT_H
#define CPU_LIMIT_CPULIMIT_H
#include <unistd.h>
#include "qh_thread/thread.h"

class CPULimit : public QH_THREAD::CThread {
  public:
    CPULimit();
    ~CPULimit();

  public:
    void init();
    void uninit();

  public:
    int setSpeed(unsigned int iSpeed, pthread_t thread);
    int monitor();

  private:
    int getCPUNum();
    int checkProc();
    int getJiffies();
    int calcateProcessCpuUsage();
    unsigned long timeDiff(const struct timeval* t1, const struct timeval* t2);

  protected:
    virtual void* thread_function(void* param);

  private:
    pid_t m_pid;
    unsigned int m_nSpeed;
    unsigned int m_nCPUNums;
    int m_nLastJiffies;
    struct timeval m_tLastSample;
    double m_dCpuUsage;
};

#endif /* CPU_LIMIT_CPULIMIT_H */
 
