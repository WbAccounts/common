// #ifndef CPU_LIMIT_TEST_H_
// #define CPU_LIMIT_TEST_H_

// #include <unistd.h>
// #include <errno.h>
// #include <dirent.h>
// #include <dlfcn.h>
// #include <stddef.h>
// #include <signal.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <string>
// #include <fstream>
// #include "common/qh_thread/multi_thread.h"

// class GetOptArgs {
//   public:
//     int m_concurrence; // 并发数量
//     int m_speed;       // 初始CPU占用率
//     GetOptArgs()
//         : m_concurrence(1)
//         , m_speed(0) {}
//     void Usage(const char *name);
//     bool GetOpt(int & argc, char* argv[]);
// };

// class UnitTest : public GetOptArgs, public QH_THREAD::CMultiThread {
//   public:
//     UnitTest() {}
//     virtual ~UnitTest();

//   public:
//     bool Init(int & argc, char* argv[]);

//   private:
//     virtual void* thread_function(void* param);
// };

// #endif /* CPU_LIMIT_TEST_H_ */
 
