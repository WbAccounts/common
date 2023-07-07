/*
 *base64编解码工具函数: 2023-01-18 created by qudreams
 */

#ifndef TQ_BASE64_UTILS_H
#define TQ_BASE64_UTILS_H

#include <sys/types.h>
#include <string>
#include <stdlib.h>

namespace base64_utils
{

    /*examples:
    *
    *编码:
    *size_t dstlen = encoded_len(srclen);
    *u_char* dst = calloc(1,dstlen);
    *encode(dst,dstlen,src,srclen);
    *free(dst);
    * 
    *解码:
    *size_t dstlen = decoded_len(srclen);
    *u_char* dst = calloc(1,dstlen);
    *int ret = decode(dst,&dstlen,src,srclen);
    *if(ret != 0) { //失败了!! }
    *free(dst);
    */

    static inline size_t encoded_len(size_t len) { 
        return (((len + 2) / 3) * 4); 
    }

    static inline size_t decoded_len(size_t len) { 
        return (((len + 3) / 4) * 3); 
    }

    //常规base64编码
    void encode(u_char *dst,size_t* dstlen, 
            const u_char *src, size_t srclen);
    //url base64编码
    void encode_url(u_char *dst,size_t* dstlen, 
            const u_char *src, size_t srclen);
    
    bool encode(std::string& dst,const std::string& src);
    bool encode_url(std::string& dst,const std::string& src);

    //常规base64解码
    //@ret: 成功返回0,失败返回-1
    int decode(u_char *dst,size_t* dstlen,
            const u_char* src,size_t srclen);
    
    //base64 url解码
    //@ret: 成功返回0,失败返回-1
    int decode_url(u_char *dst,size_t* dstlen,
                const u_char* src,size_t srclen);
    
    bool decode(std::string& dst,const std::string& src);
    bool decode_url(std::string& dst,const std::string& src);
};


#endif
