#ifndef X_PACK_H_DSMISKILLING__ST_HELP_H
#define X_PACK_H_DSMISKILLING__ST_HELP_H

#include "x2struct/x2struct.hpp"
class XPack {
public:
    // string to struct
    template <typename TYPE>
    static int DeCode(const std::string&str, TYPE&t) {
        int nRet = 0;
        try{ x2struct::X::loadjson(str,t,false); }
        catch(...){nRet = -1;}
        if (nRet) {
            printf("json_convert  String2T failed: %s\n", str.c_str());
        }
        return nRet;
    }
    //struct to string
    template <typename TYPE>
    static void EnCode(const TYPE&t, std::string&str) {
        try{ str = x2struct::X::tojson(t);}
        catch(...){}
    }
    template <typename TYPE>
    static int DeCodeFile(const std::string&str, TYPE&t) {
        int nRet = 0;
        try{ x2struct::X::loadjson(str,t,true); }
        catch(...){nRet = -1;}
        if (nRet) {
            printf("json_convert  String2T failed: %s\n", str.c_str());
        }
        return nRet;
    }
};


#endif /* X_PACK_H_DSMISKILLING__ST_HELP_H */
 
 
