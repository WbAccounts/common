#include "log_helper.h"
#include "thread/thread_utils/thread.hpp"

class CModule2 {
public:
    CModule2() {
        LOG_INFO("this is module2 constructor");
        m_thread.set_thread_func(std::tr1::bind(&CModule2::thread_function, this, std::tr1::placeholders::_1));
        m_thread.run();
    }
    ~CModule2() {}

    void* thread_function(void*) {
        for (int i=0; i<10; ++i) {
            LOG_INFO("this is module2 thread");
            sleep(1);
        }
    }
private:
    thread::CThread m_thread;
};