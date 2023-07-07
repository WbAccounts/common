#include "base64_utils.h"

namespace base64_utils {

    static void encode_internal(u_char *dst,size_t* dstlen, 
                        const u_char *src, size_t srclen, 
                        const u_char *basis,bool bpadding)
    {
        u_char         *d, *s;
        size_t          len;

        d = dst;
        len = srclen;
        s = (u_char*)src;

        while (len > 2) {
            *d++ = basis[(s[0] >> 2) & 0x3f];
            *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
            *d++ = basis[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
            *d++ = basis[s[2] & 0x3f];

            s += 3;
            len -= 3;
        }

        if (len) {
            *d++ = basis[(s[0] >> 2) & 0x3f];

            if (len == 1) {
                *d++ = basis[(s[0] & 3) << 4];
                if (bpadding) {
                    *d++ = '=';
                }

            } else {
                *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
                *d++ = basis[(s[1] & 0x0f) << 2];
            }

            if (bpadding) {
                *d++ = '=';
            }
        }

        *dstlen = d - dst;
    }

    void encode(u_char *dst,size_t* dstlen, 
            const u_char *src, size_t srclen)
    {
        static u_char   basis64[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

       encode_internal(dst,dstlen,src,srclen,basis64,true);
    }


    void encode_url(u_char *dst,size_t* dstlen, 
            const u_char *src, size_t srclen)
    {
        static u_char   basis64[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

       encode_internal(dst,dstlen,src,srclen,basis64,false);
    }

    static bool encode_internal(std::string& dst,
                    const std::string& src,bool burl)
    {
        bool ok = false;
        do {
            if(src.empty()) { break; }

            size_t dstlen = encoded_len(src.size());
            u_char* pdst = (u_char*)calloc(1,dstlen);
            if(!pdst) { break; }

            if(burl) {
                encode_url(pdst,&dstlen,
                    (const u_char*)src.data(),src.size());
            } else {
                encode(pdst,&dstlen,
                    (const u_char*)src.data(),src.size());
            }
            dst.assign((char*)pdst,dstlen);
            free(pdst);
            ok = true;
        } while(false);

        return ok;
    }

    bool encode(std::string& dst,const std::string& src)
    {
        return encode_internal(dst,src,false);
    }

    bool encode_url(std::string& dst,const std::string& src)
    {
        return encode_internal(dst,src,true);
    }

    static int decode_internal(u_char *dst, size_t* dstlen,
                            const u_char *src, size_t srclen,
                            const u_char *basis)
    {
        size_t          len;
        u_char         *d, *s;

        for (len = 0; len < srclen; len++) {
            if (src[len] == '=') {
                break;
            }

            if (basis[src[len]] == 77) {
                return -1;
            }
        }

        if (len % 4 == 1) {
            return -1;
        }

        d = dst;
        s = (u_char*)src;

        while (len > 3) {
            *d++ = (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
            *d++ = (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
            *d++ = (u_char) (basis[s[2]] << 6 | basis[s[3]]);

            s += 4;
            len -= 4;
        }

        if (len > 1) {
            *d++ = (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
        }

        if (len > 2) {
            *d++ = (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
        }

        *dstlen = d - dst;

        return 0;
    }

    //常规base64解码
    int decode(u_char *dst,size_t* dstlen,
                const u_char* src,size_t srclen)
    {
        static u_char   basis64[] = {
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
            77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
            77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
        };

        return decode_internal(dst,dstlen,src,srclen,basis64);
    }

    //base64 url解码
    int decode_url(u_char *dst,size_t* dstlen,
                const u_char* src,size_t srclen)
    {
        static u_char  basis64[] = {
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
            77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 63,
            77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
            77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
        };

        return decode_internal(dst,dstlen,src,srclen,basis64);
    }

    static bool decode_internal(std::string& dst,
                    const std::string& src,bool burl)
    {
        bool ok = false;
        do {
            if(src.empty()) { break; }

            size_t dstlen = decoded_len(src.size());
            u_char* pdst = (u_char*)calloc(1,dstlen);
            if(!pdst) { break; }

            if(burl) {
                decode_url(pdst,&dstlen,
                    (const u_char*)src.data(),src.size());
            } else {
                decode(pdst,&dstlen,
                    (const u_char*)src.data(),src.size());
            }
            dst.assign((char*)pdst,dstlen);
            free(pdst);
            ok = true;
        } while(false);

        return ok;
    }

    bool decode(std::string& dst,const std::string& src)
    {
        return decode_internal(dst,src,false);
    }

    bool decode_url(std::string& dst,const std::string& src)
    {
       return decode_internal(dst,src,true);
    }
}