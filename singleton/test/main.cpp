#include "module1.h"
#include "module2.h"

#define SINGLETON_TEMPLATE_TEST

int main () {
    LOG_HELPER->init_logger_cmd("singleton.test");
    LOG_INFO("---------------START-----------------");

    CTestClass1* test = CSingletonTemplate<CTestClass1>::getInstance();
    if (test) {
        LOG_INFO("CTestClass1::getInstance success");
        if (test == CSingletonTemplate<CTestClass1>::getInstance()) {
            LOG_INFO("CTestClass1::getInstance is equal");
        } else {
            LOG_ERROR("CTestClass1::getInstance failed");
        }
        CSingletonTemplate<CTestClass1>::destroyInstance();
    } else {
        LOG_ERROR("CTestClass::getInstance failed");
    }

    CTestClass2* test2 = CTestClass2::getInstance(1);
    if (test2) {
        LOG_INFO("CTestClass2::getInstance success");
        if (test2 == CTestClass2::getInstance(1)) {
            LOG_INFO("CTestClass2::getInstance is equal");
        } else {
            LOG_ERROR("CTestClass2::getInstance failed");
        }
        CTestClass2::destroyInstance();
    } else {
        LOG_ERROR("CTestClass2::getInstance failed");
    }
    return 0;
}