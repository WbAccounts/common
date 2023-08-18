#ifndef UTILS_STRING_UTILS_H_
#define UTILS_STRING_UTILS_H_
#include <regex.h>
#include <errno.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <tr1/memory>
#include <string.h>
#include "iconv.h"
#include <fstream>

namespace string_utils {
inline bool ToInt(const std::string& s, int& i) {
    std::stringstream ss;
    ss << s;
    ss >> i;
    return true;
}

inline int Oct2Int(const std::string& oct_str) {
    int iValue;
    ::sscanf(oct_str.c_str(), "%o", &iValue);
    return iValue;
}

inline int Hex2Int(const std::string& hex_str) {
    int iValue;
    ::sscanf(hex_str.c_str(), "%x", &iValue);
    return iValue;
}

inline std::string OctInt2String(const int ivalue) {
    char buf[128] = {0};
    ::sprintf(buf, "%o", ivalue);
    return std::string(buf);
}

inline std::string ToString(int i) {
    std::string strvalue;
    std::stringstream ss;
    ss << i;
    ss >> strvalue;
    return strvalue;
}

inline std::string& Trim(std::string& text) {
     if (!text.empty()) {
         text.erase(0, text.find_first_not_of(" \n\r\t"));
         text.erase(text.find_last_not_of(" \n\r\t") + 1);
     }
     return text;
}

inline std::string& Trim(std::string& text, const std::string& space_character_set) {
     if (!text.empty()) {
         text.erase(0, text.find_first_not_of(space_character_set));
         text.erase(text.find_last_not_of(space_character_set) + 1);
     }
     return text;
}

inline std::string& TrimLeft(std::string& text) {
     if (!text.empty()) {
         text.erase(0, text.find_first_not_of(" \n\r\t"));
     }
     return text;
}

inline std::string& TrimLeft(std::string& text, const std::string& space_character_set) {
     if (!text.empty()) {
         text.erase(0, text.find_first_not_of(space_character_set));
     }
     return text;
}

inline std::string& TrimRight(std::string& text) {
     if (!text.empty()) {
         text.erase(text.find_last_not_of(" \n\r\t") + 1);
     }
     return text;
}

inline std::string& TrimRight(std::string& text, const std::string& space_character_set) {
     if (!text.empty()) {
         text.erase(text.find_last_not_of(space_character_set) + 1);
     }
     return text;
}

inline std::string& ToUpper(std::string& s) {
    transform(s.begin(), s.end(), s.begin(), (int (*)(int))toupper);
    return s;
}

inline std::string& ToLower(std::string& s) {
    transform(s.begin(), s.end(), s.begin(), (int (*)(int))tolower);
    return s;
}

inline bool IEquals(const std::string& ls, const std::string& rs) {
    return (ls == rs);
}

enum REPLACE_TYPE {
    REPLACE_NORMAL,
    REPLACE_RECURSE,
};

template <typename T>
T& ReplaceSeq(T& input, const T& old_seq, const T& new_seq, REPLACE_TYPE replace_type = REPLACE_NORMAL) {
    if (replace_type == REPLACE_RECURSE) {
        T new_tmp = new_seq;
        typename T::iterator find_pos = std::search(new_tmp.begin(), new_tmp.end(), old_seq.begin(), old_seq.end());
        if (find_pos != new_tmp.end()) {
            fprintf(stderr, "[%s][%d] error: old_seq in new_seq, will cause a dead cycle\n", __FUNCTION__, __LINE__);
            return input;
        }
    }
    typename T::iterator cur_pos = input.begin();
    unsigned int offset = 0;
    while (cur_pos != input.end()) {
        typename T::iterator find_pos = std::search(cur_pos, input.end(), old_seq.begin(), old_seq.end());
        if (find_pos == input.end()) {
            break;
        }
        typename T::iterator last_pos = find_pos;
        for (unsigned i=0;i<old_seq.size();i++) last_pos++;

        try {
            find_pos = input.erase(find_pos, last_pos);
            offset = find_pos - input.begin();
            input.insert(find_pos, new_seq.begin(), new_seq.end());
        } catch (const std::exception &e) {
            fprintf(stderr, "[%s][%d] %s\n", __FUNCTION__, __LINE__, e.what());
            return input;
        }
        cur_pos = input.begin();
        for (unsigned i=0;i<offset;i++) cur_pos++;
        if (replace_type == REPLACE_NORMAL) {
            for (unsigned i=0;i<(new_seq.size());i++) cur_pos++;
        }
    }
    return input;
}

inline std::string& Replace(std::string& input, const std::string& old_str,
                            const std::string& new_str,
                            REPLACE_TYPE replace_type = REPLACE_NORMAL) {
    return ReplaceSeq(input, old_str, new_str, replace_type);
}

inline std::string& FormatPathSlash(std::string& path) {
    Replace(path, "/./", "/", REPLACE_RECURSE);
    Replace(path, "//", "/", REPLACE_RECURSE);
    return path;
}

inline std::string JoinPath(const std::string& ls, const std::string& rs) {
    std::string left_string = ls, right_string = rs;
    std::string pathJpined = Trim(left_string) + '/' + Trim(right_string);
    FormatPathSlash(pathJpined);
    return pathJpined;
}

template <typename T>
void Split(T& string_container, const std::string& s_to_split,
           const std::string& token) {
    string_container.clear();
    size_t p = std::string::npos, pp = 0;
    bool bFind = false;
    while ((p = s_to_split.find(token, pp)) != std::string::npos) {
        bFind = true;
        std::string s = s_to_split.substr(pp, p - pp);
        Trim(s);
        if (!s.empty()) string_container.insert(string_container.end(), s);
        while (s_to_split.substr(p + token.size(), token.size()) == token) {
            p = p + token.size();
            if (p >= (s_to_split.size() - token.size())) break;
        }
        pp = p + token.size();
    }
    if ((pp != 0 && pp < s_to_split.size()) || (bFind == false)) {
        std::string s = s_to_split.substr(pp);
        Trim(s);
        if (!s.empty()) string_container.insert(string_container.end(), s);
    }
}

static inline void IconvDeleter(iconv_t* iconv_point) { iconv_close(*iconv_point); }

typedef std::tr1::shared_ptr<char> EncodingConvertResult;
static inline EncodingConvertResult EncodingConvert(char* in, size_t in_bytes,
                                      const std::string& from_format,
                                      const std::string& to_format,
                                      size_t& out_bytes) {
    out_bytes = 0;
    iconv_t conv_open_ret = iconv_open(to_format.c_str(), from_format.c_str());
    if (conv_open_ret == (iconv_t)-1) {
        return EncodingConvertResult();
    }
    std::tr1::shared_ptr<iconv_t> conv_handle(&conv_open_ret, IconvDeleter);

    size_t in_left = in_bytes;
    size_t out_size = in_bytes * 6;  // UTF8 最多使用6个bytes表示一个字符
    size_t out_left = out_size;
    char* out_calloc_ret = (char*)calloc(1, out_left);
    if (out_calloc_ret == NULL) {
        return EncodingConvertResult();
    }
    EncodingConvertResult out(out_calloc_ret, free);
    size_t converted = iconv(conv_open_ret, &in, &in_left, &out_calloc_ret, &out_left);

    if (converted == (size_t)-1) {
        return EncodingConvertResult();
    }
    out_bytes = out_size - out_left;
    return out;
}

template <typename T>
std::string NumToString(T num) {
    std::ostringstream oss;
    oss << num;
    std::string str(oss.str());
    return str;
}

template <typename T>
T StringToNum(const std::string & str) {
    std::istringstream iss(str);
    T num = 0;
    iss >> num;
    return num;
}

template <typename F, typename T>
void Convert(const F& from, T& to) {
    std::stringstream ss;
    ss << from;
    ss >> to;
}

//这个性能更好，并且更简洁
static inline u_char* HexLowerDump(u_char *dst,
                const u_char *src, size_t len)
{
    static u_char  hex[] = "0123456789abcdef";

    while (len--) {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src++ & 0xf];
    }

    return dst;
}

//这个性能更好，并且更简洁
static inline u_char* HexUpperDump(u_char *dst,
                const u_char *src, size_t len)
{
    static u_char  hex[] = "0123456789ABCDEF";

    while (len--) {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src++ & 0xf];
    }

    return dst;
}

static int inline _bin2hex(unsigned char bin, unsigned char hex[2], unsigned int uppercase)
{
    if(uppercase) {
        HexUpperDump(hex,&bin,1);
    } else {
        HexLowerDump(hex,&bin,1);
    }
    return 0;
}

inline std::string Bin2Hex(const std::string &strBin)
{
    unsigned char buffer[3] = {0};
    std::string strHex;
    std::stringstream ss;
    for (size_t i = 0; i < strBin.length(); i++) {
        unsigned char uc = (unsigned char)strBin.at(i);
        _bin2hex(uc, buffer, 0);
        ss << buffer;
    }
    strHex = ss.str();
    return strHex;
}

inline std::string Bin2Hex(const char *buffer, size_t len)
{
    unsigned char _buffer[3] = {0};
    std::string strHex;
    std::stringstream ss;
    for (size_t i = 0; i < len; i++) {
        unsigned char uc = (unsigned char)buffer[i];
        _bin2hex(uc, _buffer, 0);
        ss << _buffer;
    }
    strHex = ss.str();
    return strHex;
}

inline bool StartsWith(const std::string &s, const std::string &sub) {
    return s.find(sub) == 0;
}

inline bool EndsWith(const std::string &s, const std::string &sub) {
    return s.rfind(sub) == (s.length() - sub.length());
}

template <typename T>
inline T HexToDec(const std::string& hex) {
    T dec;
    ::sscanf(hex.c_str(), "%llx", &dec);
    return dec;
}

inline bool IsEmptyOrNull(const char* p)
{
    return (!p || !p[0]);
}


template <typename T>
std::string Join(const T& string_container, const std::string& sep) {
    std::string res;
    if (string_container.empty()) {
        return res;
    }

    typename T::const_iterator iter = string_container.begin();
    res += *iter;

    for(++iter; iter !=string_container.end(); ++iter){
        res += sep;
        res += *iter;
    }
    return res;
}

/*
 *处理字符串行(支持\r\n或单纯以\n结尾的情况)
 *
 * @buf-> 字符串缓冲区,不能为空
 * @len-> buf的长度标识,一定要大于0
 * @premain->本次处理后的剩余的字符串开始位置，可以为空
 * @cb->获取到一行字符串后的回调，
 *    返回值不为0时调用者指示process_lines要结束处理，返回0继续处理
 *    cb的使用要特别注意，其第一个参数是返回的字符串指针且该字符串不是以\0结尾的，长度由第二个参数标识
 *    cb第三个参数ctx是调用者在调用process_lines时指定的
 * 
 * @ctx->由调用者定义及使用
 * Note:
 * 1.只所以实现这个函数，是因我们在处理字符串时很多时候不想进行copy
 * 2.
 */
static inline int process_lines(const char* buf,size_t len,char** premain,
                            int (*cb)(const char* pline,size_t linelen,void* ctx),
                            void* ctx)
{
    int cbret = 0;
    char* p = NULL;

    if(!buf || !len || !cb) {
        return -1;
    } 

    char* pstart = (char*)buf;
    char* pend = (char*)buf + len;
    do {
        p = (char*)memchr(pstart,'\n',len);
        if(p == NULL) { break; }

        {
            //回车符\r
            const char* pretkey = NULL;
            size_t line_size = (p - pstart);

            //判断是否存在\r
            if((p > pstart) && (*(p - 1) == '\r')) {
                pretkey = (p - 1);
                line_size = (pretkey - pstart);
            }

            cbret = cb(pstart,line_size,ctx);
        }

        //skip '\n'
        len -= (p - pstart + 1);

        pstart = p + 1;
        //尾部刚好是一个\n
        if(pstart >= pend) {
            break; 
        }
        
        //返回非0直接结束
        if(cbret) { break; }
    }while(p != NULL);

    if(premain) {
        if(pstart < pend) {
            *premain = pstart;
        } else {
            *premain = NULL;
        }
    }
    
    return cbret;
}

inline std::string c_regex_replace(const std::string str, const std::string& pattern, const std::string replace_str) {
    // std::string str = "https://192.168.1.1/index.html";
    // std::string pattern = "https://[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}";
    // std::string replace_str = "https://[fe80::52c5:5b1a:9aa:2158]";
    std::string strdst;

    regex_t reg;
    regcomp(&reg, pattern.c_str(), REG_EXTENDED);

    regmatch_t matches[1];
    if (regexec(&reg, str.c_str(), 1, matches, 0) == 0) {
        strdst = str.substr(0, matches[0].rm_so);
        strdst += replace_str;
        strdst += str.substr(matches[0].rm_eo);
    } else {
        strdst = str;
    }

    regfree(&reg);

    return strdst;
}

inline bool SimpleGrep(const std::string& filename, const std::string& pattern, bool ignoreCase = false, bool matchWholeWord = false) {
    std::ifstream ifs(filename.c_str());
    if (ifs.fail()) {
        return false;
    }

    std::string patternCopy = pattern;
    if (ignoreCase) {
        std::transform(patternCopy.begin(), patternCopy.end(), patternCopy.begin(), ::tolower);
    }

    std::string line;
    std::size_t pos = std::string::npos;
    while (std::getline(ifs, line)) {
        if (ignoreCase) {
            std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        }
        pos = line.find(patternCopy);
        if (pos != std::string::npos) {
            if (matchWholeWord) {
                std::size_t left = pos;
                std::size_t right = pos + pattern.length();
                if ((left == 0 || !std::isalnum(line[left - 1])) && (right == line.length() || !std::isalnum(line[right]))) {
                    return true;
                }
            } else {
                return true;
            }
        }
    }

    ifs.close();

    return false;
}

}

#endif  /* UTILS_STRING_UTILS_H_ */
 
 
