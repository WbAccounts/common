#ifndef _RAPIDJSON_UTILS_HPP_
#define _RAPIDJSON_UTILS_HPP_
#include <fstream>
#include <sys/stat.h>

#include "rapidjson_custom.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

namespace rj_utils{
inline bool loadFile(const std::string& config, rapidjson::Document& doc) {
    struct stat lsb;
    if (lstat(config.c_str(), &lsb) != 0) {
        return false;
    }
    std::ifstream ifs(config.c_str());
    rapidjson::IStreamWrapper isw(ifs);
    doc.ParseStream(isw);
    if (doc.HasParseError()) {
        return false;
    }
    return true;
}

inline bool WriteFile(const std::string& config, const rapidjson::Value& doc) {
    try {
        std::ofstream ofs(config.c_str());
        rapidjson::OStreamWrapper osw(ofs);
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
        //rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        doc.Accept(writer);
        //writer.Flush();
    } catch (...) {
        return false;
    }
    return true;
}

inline std::string toString(const rapidjson::Value& doc) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    doc.Accept(writer);
    std::string str = sb.GetString();
    return str;
}

inline bool fromString(const std::string& str, rapidjson::Document& doc) {
    doc.SetNull();
    rapidjson::ParseResult pr = doc.Parse(str.c_str());
    return !pr ? false : true;
}

inline std::string getString(const rapidjson::Value& doc, const std::string& jpath, const std::string& default_value = "") {
    if (!doc.IsNull()) {
        const rapidjson::Value* value = rapidjson::Pointer(jpath).Get(doc);
        if (value && value->IsString()) {
            return value->GetString();
        }
    }
    return default_value;
}

inline int64_t getInt64(const rapidjson::Value& doc, const std::string& jpath, int64_t default_value = 0) {
    if (!doc.IsNull()) {
        const rapidjson::Value* value = rapidjson::Pointer(jpath).Get(doc);
        if (value && value->IsInt64()) {
            return value->GetInt64();
        }
    }
    return default_value;
}

inline bool getBool(const rapidjson::Value& doc, const std::string& jpath, bool default_value = false) {
    if (!doc.IsNull()) {
        const rapidjson::Value* value = rapidjson::Pointer(jpath).Get(doc);
        if (value && value->IsBool()) {
            return value->GetBool();
        }
    }
    return default_value;
}

template <typename T1, typename T2>
void setValue(T1& doc, const std::string& jpath, T2& value) {
    rapidjson::Pointer(jpath).Set(doc, value);
}

template <typename T>
T getValue(const rapidjson::Value& doc, const std::string& jpath, const T& default_value) {
    if (!doc.IsNull()) {
        const rapidjson::Value* value = rapidjson::Pointer(jpath).Get(doc);
        if (value && value->Is<T>()) {
            return value->Get<T>();
        }
    }
    return default_value;
}

}

#endif  /* _RAPIDJSON_UTILS_HPP_ */
 
 
