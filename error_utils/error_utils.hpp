// #pragma once
// #include <map>
// #include <string.h>
// #include "common/qh_thread/thread.h"
// #include "common/singleton.hpp"

// class CError {
// public:
//     CError() {}
//     ~CError() {}

// public:
//     void set_error_code(int error_code)
//     { 
//         QH_THREAD::CMutexAutoLocker Lck(&m_mutex);
//         tid_error[pthread_self()] = error_code; 
//         return; 
//     }

//     int  get_error_code()
//     { 
//         QH_THREAD::CMutexAutoLocker Lck(&m_mutex);
//         if (tid_error.find(pthread_self()) != tid_error.end()) 
//         { 
//             return tid_error[pthread_self()];
//         }
//         return true;
//     }

//     void set_error_out(std::string error_out)
//     {
//         QH_THREAD::CMutexAutoLocker Lck(&m_mutex);
//         tid_out[pthread_self()] = error_out; 
//         return;
//     }

//     std::string get_error_out()
//     {
//         if (tid_out.find(pthread_self()) != tid_out.end()) 
//         {
//             return tid_out[pthread_self()];
//         }   
//         return "no_output";
//     }

// private:
//     QH_THREAD::CMutex m_mutex;
//     std::map<pthread_t, int> tid_error;
//     std::map<pthread_t, std::string> tid_out;
// };
// #define _ERROR_ Singleton<CError>::Instance()

// #define ERROR_EASY_IMPLEMENT()\
// public:\
//     virtual void set_error_code(int error_code) { return _ERROR_.set_error_code(error_code); }\
//     virtual int  get_error_code() { return _ERROR_.get_error_code(); }\
//     virtual void set_error_out(std::string error_out) { return _ERROR_.set_error_out(error_out); }\
//     virtual std::string get_error_out() { return _ERROR_.get_error_out(); }

// #define ERROR_EASY_IMPLEMENT(class)\
// private:\
//     virtual void set_error_code(int error_code) { return m_error_##class.set_error_code(error_code); }\
//     virtual void set_error_out(std::string error_out) { return m_error_##class.set_error_out(error_out); }\
// public:\
//     virtual int  get_error_code() { return m_error_##class.get_error_code(); }\
//     virtual std::string get_error_out() { return m_error_##class.get_error_out(); }\
// private:\
//     CError m_error_##class;