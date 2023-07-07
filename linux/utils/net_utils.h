#ifndef UTILS_NET_UTILS_H_
#define UTILS_NET_UTILS_H_

#include <map>
#include <vector>
#include <memory>
#include <string>
#include "curl/curl.h"
#include "utils/tess_proxy.hpp"
#include "common/proxy_help/global_proxy.hpp"

struct HttpHeaderInfo {
    std::string UserName;
    std::string Password;
    std::string Date;
    std::string ContentLength;
    std::string ContentType;
    std::string Connection;
    std::string Host;
    std::string Key;
};

class CNetCurl
{
    // Post form.
    struct FormParam {
        enum Type {
            DEFAULT = 0,
            CONTENT,
            FILE,
        } type;
        std::string name;
        std::string content;        // NAME_CONTENT.
        std::string file;           // FILE.
    };
  public:
    CNetCurl();
    ~CNetCurl() { UnInit(); }

  public:
    bool Init();
    bool UnInit();
    void SetUrl(const char *url);

    // Get.
    bool Get(int timeout = 60, int conntime = 60);
    bool Get(const char *filepath, const struct HttpHeaderInfo &header_info);
    bool Get(const char *filepath);
    bool GetEx(const char *filepath, const std::map<std::string, std::string> &header_info);

    //支持限速，maxConnections:最大并发连接数，maxSpeed为0表示不限制速度,单位是bytes/second,
    bool GetWithLimit(const char *filepath,long maxConnections,long maxSpeed);

    // Put.
    bool Put(const char *filepath, const struct HttpHeaderInfo &header_info);
    bool PutEx(const char *filepath, const std::map<std::string, std::string> &header_info);

    // Post.
    void AddForm(const char *name, const char *content);
    void AddFile(const char *name, const char *fullpath);
    bool Post(int timeout = 60);
    bool Post(int timeout, const char* lpbuf, int nlen, int conntime = 60);
    bool Post(int timeout,const char* lpbuf, int nlen, const std::map<std::string, std::string> &header_info, int conntime = 60);
    bool Post2(const char *formName, int timeout = 60);
    bool PostWithout100(int timeout = 60); // 移除 Expect: 100-continue

    // Delete.
    bool Delete(const struct HttpHeaderInfo &header_info, int timeout = 10 * 60, int conntime = 60);
    bool DeleteEx(const std::map<std::string, std::string> &header_info, int timeout = 10 * 60, int conntime = 60);

    // Control.
    void Stop();

    // Status.
    bool IsStopped();
    bool GetResponse(void *res, int *len);
    std::string GetStrResponse();
    bool SaveAsFile(const char *fullpath);
    bool GetRespCode(long& respcode);
    std::string GetCurlCode(int & curlcode);

    static size_t read_function(char *bufptr, size_t size, size_t nitems, void *userp);
    static size_t read_function_put_file(char *bufptr, size_t size, size_t nitems, void *userp);
    static size_t write_function(void *buffer, size_t size, size_t nmemb, void *userp);
    static size_t write_function_get_file(void *buffer, size_t size, size_t nmemb, void *userp);
    static int progress_function(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

    bool PostWithout100WithLimit(long maxSpeed, long maxConnections = 0, int timeout = 60);

  private:
    // logic.
    bool requestStop();
    bool polling(int timeout);

  private:
    // data.
    CURL *m_easy_handle_;
    CURLM *m_multi_handle_;
    // Url.
    std::string m_url_;
    // Response.
    void *m_recv_;
    int m_recv_len_;
    int m_url_code_;

    std::vector<FormParam> m_form_;

    volatile long m_request_stop_;
    bool m_is_stopped_;

  private:
    void set_proxy();
    TessProxy m_tess_proxy;
    proxy_info_helper::ProxyInfoHelper m_proxy_help;
};

#endif /* UTILS_NET_UTILS_H_ */ 
 
