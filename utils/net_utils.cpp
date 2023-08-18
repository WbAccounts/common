#include "utils/net_utils.h"
#include "log/log.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "common/utils/string_utils.hpp"

CNetCurl::CNetCurl()
    :m_easy_handle_(NULL)
    ,m_url_("")
    ,m_recv_(NULL)
    ,m_recv_len_(0)
    ,m_request_stop_(0)
    ,m_is_stopped_(true) {
}

bool CNetCurl::Init() {
    m_easy_handle_ = curl_easy_init();
    return m_easy_handle_ != NULL;
}

bool CNetCurl::UnInit() {
    Stop();

    for (int i = 0; i < 100; ++i) {
        if (IsStopped()) {
            break;
        }
        usleep(1000 * 100);
    }

    if (m_easy_handle_) {
        curl_easy_cleanup(m_easy_handle_);
        m_easy_handle_ = NULL;
    }

    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_len_ = 0;
    m_request_stop_ = 0;
    m_is_stopped_ = true;
    return true;
}

void CNetCurl::SetUrl(const char *url) {
    m_url_ = url;
}

// Get.
bool CNetCurl::Get(int timeout, int conntime) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, conntime);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, timeout);

    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_FOLLOWLOCATION, 1L);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    bool res = polling(timeout);

    m_is_stopped_ = true;
    return res;
}

// Get.
bool CNetCurl::Get(const char *filepath, const struct HttpHeaderInfo &header_info) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    FILE *hd_src = fopen(filepath, "wb");
    if (hd_src == NULL) {
        LOG_ERROR("open file error, filepath=%s, error_info=%s", filepath, strerror(errno));
        return false;
    }

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function_get_file);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, hd_src);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, 60L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, 60 * 60L);
    // Headers
    curl_slist* httpHeaders = NULL;
    std::string head_host_info = "Host: " + header_info.Host;
    std::string head_user_name = "UserName: " + header_info.UserName;
    std::string head_password = "Password: " + header_info.Password;
    httpHeaders = curl_slist_append(httpHeaders, head_host_info.c_str());
    httpHeaders = curl_slist_append(httpHeaders, head_user_name.c_str());
    httpHeaders = curl_slist_append(httpHeaders, head_password.c_str());
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);

    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPGET, 1L);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    bool res = polling(0L);

    m_is_stopped_ = true;
    if (hd_src != NULL) fclose(hd_src);
    curl_slist_free_all(httpHeaders);
    return res;
}

// Get.
bool CNetCurl::Get(const char *filepath) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    FILE *hd_src = fopen(filepath, "wb");
    if (hd_src == NULL) {
        LOG_ERROR("open file error, filepath=%s, error_info=%s", filepath, strerror(errno));
        return false;
    }

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function_get_file);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, hd_src);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, 60L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, 60 * 60L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPGET, 1L);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    bool res = polling(0L);

    m_is_stopped_ = true;
    if (hd_src != NULL) fclose(hd_src);
    return res;
}

bool CNetCurl::GetWithLimit(const char *filepath,long maxConnections,long maxSpeed)
{
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }

    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    FILE *hd_src = fopen(filepath, "wb");
    if (hd_src == NULL) {
        LOG_ERROR("open file error, filepath=%s, error_info=%s", filepath, strerror(errno));
        return false;
    }

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function_get_file);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, hd_src);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, 60L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, 60 * 60L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPGET, 1L);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    if(maxSpeed > 0) {
		curl_easy_setopt(m_easy_handle_,CURLOPT_MAX_RECV_SPEED_LARGE,(curl_off_t)maxSpeed);
    }

    if(maxConnections > 0) {
		curl_easy_setopt(m_easy_handle_,CURLOPT_MAXCONNECTS,maxConnections);
    }

    set_proxy();

    bool res = polling(0L);

    m_is_stopped_ = true;
    if (hd_src != NULL) fclose(hd_src);
    return res;
}
// Get.
bool CNetCurl::GetEx(const char *filepath, const std::map<std::string, std::string> &header_info) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    FILE *hd_src = fopen(filepath, "wb");
    if (hd_src == NULL) {
        LOG_ERROR("open file error, filepath=%s, error_info=%s", filepath, strerror(errno));
        return false;
    }

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function_get_file);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, hd_src);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, 60L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, 60 * 60L);
    // Headers
    curl_slist* httpHeaders = NULL;
    std::map<std::string, std::string>::const_iterator iter;
    for (iter = header_info.begin(); iter != header_info.end(); iter++) {
        std::string strTemp = iter->first + ": " + iter->second;
        httpHeaders = curl_slist_append(httpHeaders, strTemp.c_str());
    }
    if (httpHeaders)
        curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);

    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPGET, 1L);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    bool res = polling(0L);

    m_is_stopped_ = true;
    if (hd_src != NULL) fclose(hd_src);
    curl_slist_free_all(httpHeaders);
    return res;
}

// Put.
bool CNetCurl::Put(const char *filepath, const struct HttpHeaderInfo &header_info) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    FILE *hd_src = fopen(filepath, "rb");
    if (hd_src == NULL) {
        LOG_ERROR("open file error, filepath=%s, error_info=%s", filepath, strerror(errno));
        return false;
    }

    struct stat file_info;
    stat(filepath, &file_info);

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function_put_file);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, hd_src);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, 60L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, 60 * 60L);
    // Headers
    curl_slist* httpHeaders = NULL;
    std::string head_host_info = "Host: " + header_info.Host;
    std::string head_user_name = "UserName: " + header_info.UserName;
    std::string head_password = "Password: " + header_info.Password;
    httpHeaders = curl_slist_append(httpHeaders, head_host_info.c_str());
    httpHeaders = curl_slist_append(httpHeaders, head_user_name.c_str());
    httpHeaders = curl_slist_append(httpHeaders, head_password.c_str());
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);
    // Put Params
    curl_easy_setopt(m_easy_handle_, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_PUT, 1L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    bool res = polling(0L);

    m_is_stopped_ = true;
    if (hd_src != NULL) fclose(hd_src);
    curl_slist_free_all(httpHeaders);

    return res;
}

// Put.
bool CNetCurl::PutEx(const char *filepath, const std::map<std::string, std::string> &header_info) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    FILE *hd_src = fopen(filepath, "rb");
    if (hd_src == NULL) {
        LOG_ERROR("open file error, filepath=%s, error_info=%s", filepath, strerror(errno));
        return false;
    }

    struct stat file_info;
    stat(filepath, &file_info);

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function_put_file);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, hd_src);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, 60L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, 60 * 60L);
    // Headers
    curl_slist* httpHeaders = NULL;
    std::map<std::string, std::string>::const_iterator iter;
    for (iter = header_info.begin(); iter != header_info.end(); iter++) {
        std::string strTemp = iter->first + ": " + iter->second;
        httpHeaders = curl_slist_append(httpHeaders, strTemp.c_str());
    }
    if (httpHeaders)
        curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);
    // Put Params
    curl_easy_setopt(m_easy_handle_, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_PUT, 1L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    bool res = polling(0L);

    m_is_stopped_ = true;
    if (hd_src != NULL) fclose(hd_src);
    curl_slist_free_all(httpHeaders);

    return res;
}
// Get.
bool CNetCurl::Delete(const struct HttpHeaderInfo &header_info, int timeout, int conntime) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, conntime);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, timeout);
    // Headers
    curl_slist* httpHeaders = NULL;
    std::string head_host_info = "Host: " + header_info.Host;
    std::string head_user_name = "UserName: " + header_info.UserName;
    std::string head_password = "Password: " + header_info.Password;
    httpHeaders = curl_slist_append(httpHeaders, head_host_info.c_str());
    httpHeaders = curl_slist_append(httpHeaders, head_user_name.c_str());
    httpHeaders = curl_slist_append(httpHeaders, head_password.c_str());
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);

    curl_easy_setopt(m_easy_handle_, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(m_easy_handle_, CURLOPT_FOLLOWLOCATION, 1L);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    bool res = polling(timeout);

    m_is_stopped_ = true;
    curl_slist_free_all(httpHeaders);
    return res;
}

// Get.
bool CNetCurl::DeleteEx(const std::map<std::string, std::string> &header_info, int timeout, int conntime) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }
    m_recv_ = NULL;
    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    // Downlaod.
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, conntime);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, timeout);
    // Headers
    curl_slist* httpHeaders = NULL;
    std::map<std::string, std::string>::const_iterator iter;
    for (iter = header_info.begin(); iter != header_info.end(); iter++) {
        std::string strTemp = iter->first + ": " + iter->second;
        httpHeaders = curl_slist_append(httpHeaders, strTemp.c_str());
    }
    if (httpHeaders)
        curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);

    curl_easy_setopt(m_easy_handle_, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(m_easy_handle_, CURLOPT_FOLLOWLOCATION, 1L);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    bool res = polling(timeout);

    m_is_stopped_ = true;
    curl_slist_free_all(httpHeaders);
    return res;
}
// Post.
void CNetCurl::AddForm(const char *name, const char *content) {
    FormParam fp = {};
    fp.type = FormParam::CONTENT;
    fp.name = name;
    fp.content = content;

    m_form_.push_back(fp);
}

void CNetCurl::AddFile(const char *name, const char *fullpath) {
    FormParam fp = {};
    fp.type = FormParam::FILE;
    fp.name = name;
    fp.file = fullpath;

    m_form_.push_back(fp);
}

bool CNetCurl::Post(int timeout,const char* lpbuf, int nlen, int conntime) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }

    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, conntime);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, timeout);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1);
    // Post data
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPPOST, 1L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_POSTFIELDSIZE, nlen);
    curl_easy_setopt(m_easy_handle_, CURLOPT_POSTFIELDS, lpbuf);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    set_proxy();

    // Post.
    bool res = polling(timeout);

    m_is_stopped_ = true;
    return res;
}

bool CNetCurl::Post(int timeout,const char* lpbuf, int nlen, const std::map<std::string, std::string> &header_info, int conntime) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }

    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, conntime);
    curl_easy_setopt(m_easy_handle_, CURLOPT_TIMEOUT, timeout);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1);
    // Post data
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPPOST, 1L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_POSTFIELDSIZE, nlen);
    curl_easy_setopt(m_easy_handle_, CURLOPT_POSTFIELDS, lpbuf);

    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_slist* httpHeaders = NULL;
    std::map<std::string, std::string>::const_iterator iter;
    for (iter = header_info.begin(); iter != header_info.end(); iter++) {
        std::string strTemp = iter->first + ": " + iter->second;
        httpHeaders = curl_slist_append(httpHeaders, strTemp.c_str());
    }
    if (httpHeaders)
        curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);

    set_proxy();

    // Post.
    bool res = polling(timeout);

    curl_slist_free_all(httpHeaders);
    m_is_stopped_ = true;
    return res;
}

bool CNetCurl::Post(int timeout) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }

    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, timeout);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);
    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);
    // Post Form.
    curl_httppost *formBeg = NULL, *formEnd = NULL;
    for (std::vector<FormParam>::const_iterator it = m_form_.begin(); it != m_form_.end(); ++it) {
        switch (it->type) {
        case FormParam::CONTENT:
            curl_formadd(&formBeg, &formEnd,
                CURLFORM_COPYNAME, it->name.c_str(),
                CURLFORM_COPYCONTENTS, it->content.c_str(),
                CURLFORM_END);
            break;

        case FormParam::FILE:
            curl_formadd(&formBeg, &formEnd,
                CURLFORM_COPYNAME, it->name.c_str(),
                CURLFORM_FILE, it->file.c_str(),
                CURLFORM_END);
            break;

        default:
            break;
        }
    }
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPPOST, formBeg);

    set_proxy();

    // Post.
    bool res = polling(timeout);

    // Free.
    curl_formfree(formBeg);
    formBeg = formEnd = NULL;

    m_is_stopped_ = true;
    return res;
}

bool CNetCurl::Post2(const char *formName, int timeout)
{
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }

    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, timeout);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);
    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);
    // Post Form.
    curl_httppost *formBeg = NULL, *formEnd = NULL;
    for (std::vector<FormParam>::const_iterator it = m_form_.begin(); it != m_form_.end(); ++it) {
        switch (it->type) {
        case FormParam::CONTENT:
            curl_formadd(&formBeg, &formEnd,
                CURLFORM_COPYNAME, it->name.c_str(),
                CURLFORM_COPYCONTENTS, it->content.c_str(),
                CURLFORM_END);
            break;
        case FormParam::FILE:
            curl_formadd(&formBeg, &formEnd,
                CURLFORM_COPYNAME, formName,
                CURLFORM_FILENAME, it->name.c_str(),
                CURLFORM_FILE, it->file.c_str(),
                CURLFORM_END);
            break;
        default:
            break;
        }
    }
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPPOST, formBeg);

    set_proxy();

    // Post.
    bool res = polling(timeout);
    // Free.
    curl_formfree(formBeg);
    formBeg = formEnd = NULL;

    m_is_stopped_ = true;
    return res;
}

bool CNetCurl::PostWithout100(int timeout) {
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }

    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, timeout);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);
    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_slist* httpHeaders = NULL;
    httpHeaders = curl_slist_append(httpHeaders, "Expect:");
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);

    // Post Form.
    curl_httppost *formBeg = NULL, *formEnd = NULL;
    for (std::vector<FormParam>::const_iterator it = m_form_.begin(); it != m_form_.end(); ++it) {
        switch (it->type) {
        case FormParam::CONTENT:
            curl_formadd(&formBeg, &formEnd,
                CURLFORM_COPYNAME, it->name.c_str(),
                CURLFORM_COPYCONTENTS, it->content.c_str(),
                CURLFORM_END);
            break;

        case FormParam::FILE:
            curl_formadd(&formBeg, &formEnd,
                CURLFORM_COPYNAME, it->name.c_str(),
                CURLFORM_FILE, it->file.c_str(),
                CURLFORM_END);
            break;

        default:
            break;
        }
    }
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPPOST, formBeg);

    set_proxy();

    // Post.
    bool res = polling(timeout);

    // Free.
    curl_formfree(formBeg);
    formBeg = formEnd = NULL;

    m_is_stopped_ = true;
    return res;
}

bool CNetCurl::PostWithout100WithLimit(long maxSpeed, long maxConnections, int timeout)
{
    if (m_recv_) {
        free(m_recv_);
        m_recv_ = NULL;
    }

    m_recv_len_ = 0;

    m_request_stop_ = 0;
    m_is_stopped_ = false;

    curl_easy_reset(m_easy_handle_);
    // Read.
    curl_easy_setopt(m_easy_handle_, CURLOPT_READFUNCTION, &CNetCurl::read_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_READDATA, this);
    // Write.
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEFUNCTION, &CNetCurl::write_function);
    curl_easy_setopt(m_easy_handle_, CURLOPT_WRITEDATA, this);
    // Progress.
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOPROGRESS, 1L);
    // Url.
    curl_easy_setopt(m_easy_handle_, CURLOPT_URL, m_url_.c_str());
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_CONNECTTIMEOUT, timeout);
    //
    curl_easy_setopt(m_easy_handle_, CURLOPT_NOSIGNAL, 1L);
    // not verify CA because the process compile by ubuntu qbuild always
    // verify false
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_easy_handle_, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_slist* httpHeaders = NULL;
    httpHeaders = curl_slist_append(httpHeaders, "Expect:");
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPHEADER, httpHeaders);

    // Post Form.
    curl_httppost *formBeg = NULL, *formEnd = NULL;
    for (std::vector<FormParam>::const_iterator it = m_form_.begin(); it != m_form_.end(); ++it) {
        switch (it->type) {
        case FormParam::CONTENT:
            curl_formadd(&formBeg, &formEnd,
                CURLFORM_COPYNAME, it->name.c_str(),
                CURLFORM_COPYCONTENTS, it->content.c_str(),
                CURLFORM_END);
            break;

        case FormParam::FILE:
            curl_formadd(&formBeg, &formEnd,
                CURLFORM_COPYNAME, it->name.c_str(),
                CURLFORM_FILE, it->file.c_str(),
                CURLFORM_END);
            break;

        default:
            break;
        }
    }
    curl_easy_setopt(m_easy_handle_, CURLOPT_HTTPPOST, formBeg);

    if (maxSpeed > 0) {
        curl_easy_setopt(m_easy_handle_, CURLOPT_MAX_SEND_SPEED_LARGE, (curl_off_t)maxSpeed);
    }
    if (maxConnections > 0) {
        curl_easy_setopt(m_easy_handle_, CURLOPT_MAXCONNECTS, maxConnections);
    }

    set_proxy();

    // Post.
    bool res = polling(timeout);

    // Free.
    curl_formfree(formBeg);
    formBeg = formEnd = NULL;

    m_is_stopped_ = true;
    return res;
}

// Control.
void CNetCurl::Stop() {
    m_request_stop_ = 1;
}

// Status.
bool CNetCurl::IsStopped() {
    return m_is_stopped_;
}

// Response.
bool CNetCurl::GetResponse(void *buf, int *len) {
    if (m_recv_ == NULL) {
        return false;
    }

    bool res = false;
    if (buf == NULL) {
        if (len != NULL) {
            *len = m_recv_len_;
        }
    } else if (len != NULL) {
        memset(buf, 0, *len);
        if (*len >= m_recv_len_) {
            *len = m_recv_len_;
            res = true;
        }
        memcpy(buf, m_recv_, *len);
    }

    return res;
}

bool CNetCurl::GetRespCode(long& respcode) {
    return curl_easy_getinfo(m_easy_handle_, CURLINFO_RESPONSE_CODE, &respcode);
}

std::string CNetCurl::GetCurlCode(int & curlcode) {
    curlcode = m_url_code_;
    return std::string(curl_easy_strerror((CURLcode)curlcode));
}

std::string CNetCurl::GetStrResponse() {
    std::string res;

    if (m_recv_ != NULL && m_recv_len_ != 0) {
        res.assign((char *)m_recv_, m_recv_len_);
    }

    return res;
}

bool CNetCurl::SaveAsFile(const char *fullpath) {
    return false;
}

// logic.
bool CNetCurl::requestStop() {
    if (m_request_stop_ != 0)
        return true;
    return false;
}

bool CNetCurl::polling(int timeout) {
    m_url_code_ = curl_easy_perform(m_easy_handle_);
    if (m_url_code_ != CURLE_OK) {
        LOG_ERROR("curl perform failed, errorcode[%d].", m_url_code_);
        return false;
    }

    long rescode = 0;
    curl_easy_getinfo(m_easy_handle_, CURLINFO_RESPONSE_CODE, &rescode);
    if (rescode >= 400 || rescode == 0) {
        LOG_ERROR("curl perform response code[%d] invalid.", rescode);
        return false;
    }
    return true;
}

// Static functions.
size_t CNetCurl::read_function(char *bufptr, size_t size, size_t nitems, void *userp) {
    int bufLen = size * nitems;

    return bufLen;
}

size_t CNetCurl::read_function_put_file(char *bufptr, size_t size, size_t nitems, void *userp) {
    curl_off_t nread;
    FILE *fp = (FILE*)userp;
    /* in real-world cases, this would probably get this data differently
        as this fread() stuff is exactly what the library already would do
        by default internally */ 
    size_t retcode = fread(bufptr, size, nitems, fp);
    if (ferror(fp)) {
        LOG_ERROR("fread error, errno=[%d], reason=[%s]", errno, strerror(errno));
        return CURL_READFUNC_ABORT;
    }
    nread = (curl_off_t)retcode;

    fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
            " bytes from file\n", nread);
    return retcode;

}

size_t CNetCurl::write_function(void *buffer, size_t size, size_t nmemb, void *userp) {
    int bufLen = size * nmemb;

    if (CNetCurl *p = (CNetCurl *)userp) {
        unsigned char *pOldRecv = (unsigned char *)p->m_recv_,
                      *pNewRecv = (unsigned char *)malloc(p->m_recv_len_ + bufLen);
        if (pOldRecv != NULL && pNewRecv != NULL) {
            memcpy(pNewRecv, pOldRecv, p->m_recv_len_);
        }
        if (pNewRecv != NULL) {
            memcpy(pNewRecv + p->m_recv_len_, buffer, bufLen);
        }

        if (p->m_recv_)
            free(p->m_recv_);

        p->m_recv_ = NULL;
        p->m_recv_ = pNewRecv;
        p->m_recv_len_ += bufLen;
    }

    return bufLen;
}

size_t CNetCurl::write_function_get_file(void *buffer, size_t size, size_t nmemb, void *userp) {
    size_t written;
    written = fwrite(buffer, size, nmemb, (FILE *)userp);
    return written;
}

static int32_t _to_curl_type(proxy_info_helper::ProxyInfo::ProxyType type) {
    switch (type) {
        case proxy_info_helper::ProxyInfo::HTTP:
            return CURLPROXY_HTTP;  //
        case proxy_info_helper::ProxyInfo::SOCKS5:
            return CURLPROXY_SOCKS5;  //
        default:
            return -1;
    }
    return -1;
};

void CNetCurl::set_proxy() {
    std::string strSocks5Proxy = m_tess_proxy.GetSocks5Proxy();
    proxy_info_helper::ProxyInfo proxy_info;
    if(!strSocks5Proxy.empty()) {
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXY, strSocks5Proxy.c_str());
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYUSERNAME, "xinchuang");
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYPASSWORD, m_tess_proxy.GetPassword("xinchuang").c_str());
        return;
    } else if (m_proxy_help.GetCurrentProxyInfo(proxy_info) && proxy_info.enable_) {
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXY, (proxy_info.ip_ + ":" 
                        + string_utils::NumToString<int32_t>(proxy_info.port_)).c_str());
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYTYPE, _to_curl_type(proxy_info.type_));
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYUSERNAME, proxy_info.user_.c_str());
        curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYPASSWORD, proxy_info.password_.c_str());
        return;
    }
    
    curl_easy_setopt(m_easy_handle_, CURLOPT_PROXY, "");
    curl_easy_setopt(m_easy_handle_, CURLOPT_PROXYTYPE, -1L);
}
 
 
