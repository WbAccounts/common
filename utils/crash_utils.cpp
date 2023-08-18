
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>

#include "crash_utils.h"
#include "log/log.h"
#include "file_utils.h"
#include "proc_info_utils.h"

#if defined(__sw_64__) || defined(__loongarch64)
#include <execinfo.h>
#else
#include "breakpad_wrapper/breakpad_client_init.h"
#endif


namespace crash_utils {

#if defined(__sw_64__) || defined(__loongarch64)
static int dump_fd = -1;
const int kSignals[] = {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS};
const int kNumHandledSignals = sizeof(kSignals) / sizeof(kSignals[0]);
struct sigaction oldHandlers[kNumHandledSignals];

static bool installHandlerStack() {
    void *stack_mem = malloc(2 * SIGSTKSZ);
    if (stack_mem == NULL) {
        LOG_ERROR("Signal handler Stack memory alloc failed");
        return false;
    }

    //预留栈空间
    stack_t ss;
    ss.ss_sp = stack_mem;
    ss.ss_flags = 0;
    ss.ss_size = 2 * SIGSTKSZ;
    if (sigaltstack(&ss, NULL) != 0)  {
        LOG_ERROR("Set signal handler stack memory failed");
        return false;
    }

    return true;
}

static void restoreHandlers() {
  for (int i = 0; i < kNumHandledSignals; ++i) {
    if (sigaction(kSignals[i], &oldHandlers[i], NULL) == -1) {
        signal(kSignals[i], SIG_DFL);
    }
  }
}

static void crashHandler(int n, siginfo_t *siginfo, void *myact) {
    void *array[100];
    size_t size;

    int sig = siginfo->si_signo;
    char buf[80] = {0};
    snprintf(buf, sizeof(buf), "[Backtrace] Programme recv signal [%d]:\n", sig);
    ::write(dump_fd, buf, strlen(buf));

    // get void*'s for all entries on the stack
    size = backtrace(array, 100);

    // print out all the frames to stderr
    if (dump_fd == -1) { dump_fd = 2; }
    backtrace_symbols_fd(array, size, dump_fd);

    int mapfd = open("/proc/self/maps", O_RDONLY);
    if (mapfd != -1) {
        write(dump_fd, "\nMemory map:\n", 14);

        char buf[256];
        ssize_t n;

        while ((n = TEMP_FAILURE_RETRY(read(mapfd, buf, sizeof(buf)))) > 0)
        TEMP_FAILURE_RETRY(write(dump_fd, buf, n));

        close(mapfd);
    }

    restoreHandlers();
}

bool backtraceInit(const std::string& path) {
    //生成文件名
    char file_name[80] = {0};
    srand(time(NULL));
    snprintf(file_name, sizeof(file_name), "backtrace-%d.dmp", rand());

    //预留描述符
    std::string dump_file = path + "/" + file_name;
    dump_fd = ::open(dump_file.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0644);
    if (dump_fd < 0) {
        LOG_ERROR("Dump file open failed");
        return false;
    }

    //获取旧的信号处理函数
    for (int i = 0; i < kNumHandledSignals; ++i) {
        if (sigaction(kSignals[i], NULL, &oldHandlers[i]) == -1) {
            LOG_ERROR("Store old handlers failed.");
            return false;
         }
    }

    //安装信号处理专用栈
    if (!installHandlerStack()) { return false; }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK | SA_SIGINFO;
    sa.sa_sigaction = crashHandler;

    //安装信号处理函数
    for (int i = 0; i < kNumHandledSignals; ++i) {
        if (sigaction(kSignals[i], &sa, NULL) == -1) {
            LOG_ERROR("Signal [%d] handler isntall failed.", kSignals[i]);
            return false;
        }
    }

    return true;
}
#endif

bool Init() {
    std::string dump_path = proc_info_utils::GetInstallPath() + "/Log/crash";
    return Init(dump_path);
}

bool Init(const std::string &str_crash_path) {
    bool res = false;
    std::string dump_path = str_crash_path;

    if(!file_utils::IsExist(dump_path)) {
        file_utils::MakeDirs(dump_path);
    }

// 申威只支持backtrace
#if defined(__sw_64__) || defined(__loongarch64)
    res = backtraceInit(dump_path);
#else
    res = google_breakpad::ClientInit(dump_path.c_str());
#endif
    return res;
}

} // namespace crash_utils
 
 
