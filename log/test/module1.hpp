#include "log_helper.h"
#include "thread/thread_utils/thread.hpp"

class CModule1 {
public:
    CModule1() {
        LOG_INFO("this is module1 constructor");
        m_thread.set_thread_func(std::tr1::bind(&CModule1::thread_function, this, std::tr1::placeholders::_1));
        m_thread.run();
    }
    ~CModule1() {}

    void* thread_function(void*) {
        for (int i=0; i<10; ++i) {
            LOG_INFO("this is module1 thread");
            sleep(1);
        }
    }
private:
    thread::CThread m_thread;
};