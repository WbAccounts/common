#ifndef UTILS_TESS_PROXY_HPP_
#define UTILS_TESS_PROXY_HPP_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <time.h>
#include <string>

#include "proc_info_utils.h"
#include "ASFramework/util/ASJsonWrapper.h"

#define TOTP_T0 0
#define TOTP_DIGITS 6
#define TOTP_VALIDITY 60

class RFC4226 {
    uint32_t DT(uint8_t *digest) {
        uint64_t offset;
        uint32_t bin_code;

    #ifdef DEBUG
        char mdString[40];
        for (int i = 0; i < 20; i++)
            sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
        printf("HMAC digest: %s\n", mdString);

    #endif

        // dynamically truncates hash
        offset   = digest[19] & 0x0f;

        bin_code = (digest[offset]  & 0x7f) << 24
            | (digest[offset+1] & 0xff) << 16
            | (digest[offset+2] & 0xff) <<  8
            | (digest[offset+3] & 0xff);

        // truncates code to 6 digits
    #ifdef DEBUG
        printf("OFFSET: %ld\n", offset);
        printf("\nDBC1: %d\n", bin_code);
    #endif

        return bin_code;
    }


    uint32_t mod_hotp(uint32_t bin_code, int digits) {
        int power = (int)pow(10.0, (double)digits);
        uint32_t otp = bin_code % power;
        return otp;
    }

  public:
    uint32_t HOTP(uint8_t *key, size_t kl, uint64_t interval, int digits) {
        uint32_t result = 0;
        uint32_t endianness = 0xdeadbeef;

        if ((*(const uint8_t *)&endianness) == 0xef) {
            interval = ((interval & 0x00000000ffffffffULL) << 32) | ((interval & 0xffffffff00000000ULL) >> 32);
            interval = ((interval & 0x0000ffff0000ffffULL) << 16) | ((interval & 0xffff0000ffff0000ULL) >> 16);
            interval = ((interval & 0x00ff00ff00ff00ffULL) <<  8) | ((interval & 0xff00ff00ff00ff00ULL) >>  8);
        };

        //First Phase, get the digest of the message using the provided key ...
        uint8_t *digest = (uint8_t *)HMAC(EVP_sha1(), key, kl, (const unsigned char *)&interval, sizeof(interval), NULL, 0);
        if (!digest) { return result; }
        //Second Phase, get the dbc from the algorithm
        uint32_t dbc = DT(digest);
        //Third Phase: calculate the mod_k of the dbc to get the correct number
        result = mod_hotp(dbc, digits);

        return result;
    }

};

/******** RFC6238 **********
 *
 * TOTP = HOTP(k,T) where
 * K = the supersecret key
 * T = ( Current Unix time - T0) / X
 * where X is the Time Step
 *
 * *************************/
class RFC6238 : public RFC4226 {
    uint32_t TOTP(uint8_t *key, size_t kl, uint64_t time, int digits) {
        uint32_t totp;
        totp = HOTP(key, kl, time, digits);
        return totp;
    }
  public:
    uint32_t totp(uint8_t *k, size_t keylen) {
        uint64_t t = (uint64_t)floor((time(NULL) - TOTP_T0) / TOTP_VALIDITY);
        return TOTP(k, keylen, t, TOTP_DIGITS);
    }
};

class TessProxy : public RFC6238 {
  public:
    TessProxy() {
        m_bProxy = GetProxyInfo();
    }

    std::string GetPassword(const std::string& username = "xinchuang") {
         if (!m_bProxy || username.empty()) {
            return "";
        }
        int count = 16 / username.length() + 1;
        std::string secret;
        for (int i = 0; i < count; i++) {
            secret += username;
        }
        secret = secret.substr(0, 16);

        char passwd[7] = {0};
        snprintf(passwd, 7, "%06u", totp((uint8_t*)secret.c_str(), 16));
        return std::string(passwd, 6);
    }

    std::string GetSocks5Proxy() { return m_strSocks5Proxy; }

  private:
    bool GetProxyInfo() {
        std::string tess_proxy = proc_info_utils::GetInstallPath() + "/conf/tess_proxy.conf";
        do {
            if (access(tess_proxy.c_str(), F_OK)) {
                break;
            }
            Json::Value jvRoot;
            if (!CASJsonWrapper::LoadJsonFile(tess_proxy.c_str(), jvRoot)) {
                printf("load json file %s failed\n", tess_proxy.c_str());
                break;
            }
            // {"socks5": "127.0.0.1:36440"}
            m_strSocks5Proxy = CASJsonWrapper::GetJsonValueString("socks5", jvRoot, "");
            if (!m_strSocks5Proxy.empty()) {
                return true;
            }
        } while (false);
        return false;
    }

  private:
    bool m_bProxy;
    std::string m_strSocks5Proxy;
};

#endif /* UTILS_TESS_PROXY_HPP_ */
 
 
