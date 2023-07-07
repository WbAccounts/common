/*
* Copyright (C) 2019 YY Inc. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

#ifndef __X_TYPES_H
#define __X_TYPES_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#define __X_DEF_XOPEN
#endif

#include "xstdint.h"
#include <time.h>
#include <string>
#include <stdexcept>

#ifdef __X_DEF_XOPEN
#undef _XOPEN_SOURCE
#endif

namespace x2struct {

struct x_condition_t {
    typedef bool (*cond_f)(void*obj, void*doc);

    void* parent;
    cond_f  cond;

    x_condition_t():parent(0),cond(0){}
    void set(void*p, cond_f c){parent=p; cond=c;}
};

template<typename TYPE>
class XType:public TYPE {
public:
    x_condition_t __x_cond;
    template<class DOC>
    void __x_to_struct(DOC& obj) {
        std::string str;
        obj.convert(NULL, str);
        this->parse(str);
    }
    template<class DOC>
    void __struct_to_str(DOC& obj, const char*key) const {
        std::string str = this->format();
        obj.convert(key, str);
    }
//     TYPE* operator->() {
//         return &_t;
//     }
// private:
//     TYPE _t;
};

// YYYY-MM-DD HH:MM:SS
class _XDate {
public:
    int64_t unix_time;   // unix time
public:
    std::string format() const {
        time_t tt = (time_t)unix_time;
        tm     ttm;
        localtime_r(&tt, &ttm);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ttm);
        return buf;
    }
    void parse(const std::string&str) {
        tm ttm;

        if (0 != strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &ttm)) {
            unix_time = mktime(&ttm);
        } else {
            std::string err("invalid time string[");
            err.append(str).append("]. use format YYYY-mm-dd H:M:S. ");
            throw std::runtime_error(err);
        }
    }
};

typedef XType<_XDate> XDate;

}

#endif
 
 
