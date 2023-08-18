// #include "CPULimitTest.h"
// #include <fcntl.h>
// #include <malloc.h>
// #include <dlfcn.h>
// #include <signal.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/file.h>
// #include "common/singleton.hpp"
// #include "common/utils/proc_info_utils.h"
// #include "common/ASFramework/util/ASLogImpl.h"
// #include "common/log/log.h"
// #include "common/qh_thread/multi_thread.h"
// #include "common/cpu_limit/cpu_limit_mgr.h"

// static bool RunLoop = true;
// static unsigned timeIntervalSeconds = 1;
// static UnitTest *unittest = new (std::nothrow) UnitTest;

// void GetOptArgs::Usage(const char *name) {
//     fprintf(stderr, "input argv: %s \
// \n -n <concurrence numbers> \
// \n -s <speed>\n", name);
// }

// bool GetOptArgs::GetOpt(int & argc, char* argv[]){
//     const char *args = "hn:s:";
//     extern char *optarg;
//     int rt;
//     while ((rt = getopt(argc, argv, args)) != -1) {
//         switch (rt) {
//             case 'h':
//                 Usage(argv[0]);
//                 return false;
//             case 'n':
//                 m_concurrence = atoi(optarg);
//                 break;
//             case 's':
//                 m_speed = atoi(optarg);
//                 break;
//             default:
//                 Usage(argv[0]);
//                 return false;
//         }
//     }
//     return true;
// }

// bool UnitTest::Init(int & argc, char* argv[]) {
//     if (!GetOpt(argc, argv)) {
//         return false;
//     }
//     return true;
// }

// UnitTest::~UnitTest() {
// }

// void* UnitTest::thread_function(void* param) {
//     LOG_INFO("[%ld] scan thread start.", (long)proc_info_utils::GetTid());
//     while (RunLoop) {
//         Singleton<CpuLimitManager>::Instance().SpeedCtrl();
//     }
//     LOG_INFO("[%ld] scan thread exit.", (long)proc_info_utils::GetTid());
//     return NULL;
// }

// static void signal_exit_handler(int n, siginfo_t *siginfo, void *myact) {
//     LOG_INFO("recv sig = [%d] from pid = [%d], main process exit.", n, siginfo->si_pid);
//     int sig = siginfo->si_signo;
//     if (sig == SIGUSR1) {
//         Singleton<CpuLimitManager>::Instance().SetCpuLimitMode(0);
//     } else if (sig == SIGSEGV) {
//         Singleton<CpuLimitManager>::Instance().SetCpuLimitMode(1);
//     } else if (sig == SIGUSR2) {
//         Singleton<CpuLimitManager>::Instance().SetCpuLimitMode(2);
//     } else {
//         RunLoop = false;
//     }
// }

// static void init_signals() {
//     // 先清理一下信号掩码，避免继承启动者的信号掩码值而引起问题
//     // 默认情况下我们不阻塞任何信号,对于我们要处理的信号下面再设置
//     sigset_t sigset;
//     sigemptyset(&sigset);
//     sigprocmask(SIG_SETMASK, &sigset, NULL);

//     // 忽略SIGPIPE
//     signal(SIGPIPE, SIG_IGN);

//     struct sigaction act;
//     sigemptyset(&act.sa_mask);
//     act.sa_flags = SA_SIGINFO;
//     act.sa_sigaction = signal_exit_handler;
//     int rc = sigaction(SIGUSR1, &act, NULL);
//     if (rc < 0) {
//         LOG_ERROR("install SIGUSR1 signal handler failed, because: %s.", strerror(errno));
//     }
//     rc = sigaction(SIGSEGV, &act, NULL);
//     if (rc < 0) {
//         LOG_ERROR("install SIGSEGV signal handler failed, because: %s.", strerror(errno));
//     }
//     rc = sigaction(SIGUSR2, &act, NULL);
//     if (rc < 0) {
//         LOG_ERROR("install SIGUSR2 signal handler failed, because: %s.", strerror(errno));
//     }
// }

// static void init_log() {
//     std::string strLogPath = proc_info_utils::GetInstallPath() + "/cpu_limit.log";
//     CASLogImpl* pLogger = new (std::nothrow) CASLogImpl();
//     pLogger->AddRef();
//     pLogger->SetLogFilePath(strLogPath.c_str());
//     pLogger->SetLogMaxSize(10 * 1024 * 1024);
//     pLogger->SetLogLevel(ASLog_Level_Debug);
//     pLogger->Open();
//     CEntModuleDebug::SetModuleDebugger(pLogger);
//     LOG_DEBUG("<------------------------------------>");
//     LOG_DEBUG("----> init cpu limit log success <----");
// }

// int main(int argc, char* argv[])
// {
//     init_log();
//     init_signals();
//     // unit test
//     if (!unittest->Init(argc, argv)) {
//         return -1;
//     }
//     Singleton<CpuLimitManager>::Instance().Init();
//     Singleton<CpuLimitManager>::Instance().SetCpuLimitSpeed(unittest->m_speed);
//     unittest->SetConcurrentSize(unittest->m_concurrence);
//     unittest->Run();
//     while (unittest->IsRunning()) {
//         usleep(1 * 1000);
//     }
//     unittest->SynStop();
//     if (unittest) delete unittest;
//     return 0;
// } 
