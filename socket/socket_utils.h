#ifndef COMMON_SOCKET_SOCKET_UTILS_H_
#define COMMON_SOCKET_SOCKET_UTILS_H_

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include "json/cJSON.h"
#include "socket/socket_process_info.h"

class IASBundle;

namespace socket_control {
    int GetJsonItemInfo(cJSON *object, const char *key, std::string &value, int type);
    int GetJsonItemInfo(cJSON *object, const char *key, unsigned int &value, int type);
    int GetJsonItemInfo(cJSON *object, const char *key, int &value, int type);
    int GetJsonItemInfo(cJSON *object, const char *key, bool &value, int type);
    int ConvertRecvStrToBundle(IASBundle** pSocketData, const std::string& recv_data);

    int GetBundleItemInfo(IASBundle* pBundle, const char *key, unsigned char * &value);
    int GetBundleItemInfo(IASBundle* pBundle, const char *key, std::string &value);

    int GetBundleItemInfo(IASBundle* pBundle, const char *key, unsigned int &value);
    int GetBundleItemInfo(IASBundle* pBundle, const char *key, int &value);
    int GetBundleItemInfo(IASBundle* pBundle, const char *key, int64_t &value);

    int ParseRecvBundleData(IASBundle* pSocketData, struct UnixSocketData &recvData);

    int CreateSendData(std::string& send_data, const struct UnixSocketData &recvData);
    int CreateSendData(std::string& send_data, IASBundle *pSocketData);

    int ParseSendJsonData(const std::string& send_data, struct UnixSocketData& sendData);
    std::string getBundleBinaryInfo(IASBundle *pData, const char *str_key);
    std::string GetBundleStringInfo(IASBundle* pData, const char* str_key);
    std::string GetJsonStringInfo(const std::string& json_data, const char * str_key);
    bool GetJsonIntInfo(const std::string& json_data, const char * str_key, int &nValue);

    bool GetBundleItem2(IASBundle* pBundle, const char *key,u_char* &value);
    bool GetBundleItem2(IASBundle* pBundle, const char *key,std::string &value);
    bool GetBundleItem2(IASBundle* pBundle, const char *key, unsigned int &value);
    bool GetBundleItem2(IASBundle* pBundle, const char *key, int &value);
    bool GetBundleItem2(IASBundle* pBundle, const char *key, int64_t &value);
    bool GetBundleItem2(IASBundle* pBundle, const char *key, bool &value);
}

#endif /* COMMON_SOCKET_SOCKET_UTILS_H_ */
 
 
