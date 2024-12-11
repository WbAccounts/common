#include "log/log_helper.h"
#include "singleton/singleton.hpp"

class CTestClass1 {
public:
    CTestClass1() { LOG_INFO("CTestClass1 constructor"); }
    ~CTestClass1() { LOG_INFO("CTestClass1 destructor"); }
};