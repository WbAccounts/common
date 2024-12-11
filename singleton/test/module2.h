#include "singleton/singleton.hpp"
#include "log/log_helper.h"

class CTestClass2 : public CSingletonTemplate<CTestClass2, int> {
public:
    CTestClass2(int a) { LOG_INFO("CTestClass2 constructor {}", a); }
    ~CTestClass2() { LOG_INFO("CTestClass2 destructor"); }
};