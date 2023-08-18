#ifndef UTILS_FILE_UTILS_H_
#define UTILS_FILE_UTILS_H_

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <tr1/memory>
#include <vector>
#include "utils/string_utils.hpp"

namespace file_utils {
enum FOLLOW_LINK_TYPE { FOLLOW_LINK, NOT_FOLLOW_LINK };
bool IsFile(const std::string& file_path,
            FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
bool IsDir(const std::string& file_path,
           FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
bool IsExist(const std::string& file_path,
             FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
bool IsSymLink(const std::string& file_path, bool must_effective = false);
bool FollowLink(const std::string& link, std::string& real);
bool FollowLink2(const char* link, std::string& real);

int64_t GetFileSize(const std::string& file_path,
                    FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);

bool MakeDirs(const std::string& dir, const mode_t mode = 0755);

/*
*GetParentDir("/") == "/"
*这个函数有非常严重的局限性，只是非常简单的字符串截取
*根本处理不了像.,../这种相对路径的场景
*能不用就不要再用了。直接使用dirname( man 3 dirname)多好
*/ 
std::string GetParentDir(const std::string& file_path);

//使用dirname(man 3 dirname)获取父路径
std::string GetParentDir2(const std::string& file_path);

std::string GetFileName(const std::string& file_path);
// test.tar.gz => gz
std::string GetFileSuffix(const std::string& file_path);
// test.1.tar.gz => 1.tar.gz
std::string GetFileCompleteSuffix(const std::string& file_path);
// The base name consists of all characters in the file up to (but not
// including) the first '.' character.
std::string GetBaseName(const std::string& file_path);
bool IsReadable(const std::string& file_path);
bool IsWritable(const std::string& file_path);
bool IsExecuteble(const std::string& file_path);
time_t LastModified(const std::string& file_path,
                    FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
std::string FileTimeToStr(time_t time);
std::string LastModifiedStr(
    const std::string& file_path,
    FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
uid_t GetOwnerId(const std::string& file_path,
                 FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
std::string GetOwner(const std::string& file_path,
                     FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
long GetActiveFileNumber();

bool CopyFile(const std::string& from, const std::string& to);
bool CopyFile(const std::string& from, const std::string& to, mode_t mode);
bool MoveFile(const std::string& from, const std::string& to);
bool MoveFile(const std::string& from, const std::string& to, mode_t mode);
bool CopyDir(const std::string& from, const std::string& to, mode_t mode);
bool MoveDir(const std::string& from, const std::string& to);

// return false is not dir
bool RemoveDirs(const std::string& dir,
                FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
bool RemoveFile(const std::string& file_path,
                FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
using string_utils::JoinPath;
using string_utils::FormatPathSlash;

typedef std::tr1::shared_ptr<void> FileContentSmartPtr;
FileContentSmartPtr GetFileContent(
    const std::string& file_path, size_t& size,
    FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
//外部保证buf大小足够容纳file的内容
bool GetFileContent(const std::string& file_path, int64_t file_size, char* buf,
                    FOLLOW_LINK_TYPE follow_link_type = NOT_FOLLOW_LINK);
int GetPathMaxSize(const std::string &path);
void SetFDCLOEXEC(int fd);
void SetFDNONBLOCK(int fd);
std::string MakeTempDir(const std::string& strTemplate = "/tmp/.tempdir_XXXXXX");

//这个枚举目录的函数处理不了自己软链接到自己的情况，并且无法去重
bool EnumDir(const std::string &dir,
        bool (*cb)(const char* fpath,const struct stat* st,void* ctx),
        void* ctx);

//跟dirname功能相同，使用时一定要注意一下，
//可以 man 3 dirname看一下
bool GetDirName(const char* fpath,
                std::string& dname);

//跟basename功能相同，使用时一定要注意一下，
//可以 man 3 basename看一下
bool GetBaseName2(const char* fpath,
                std::string& dname);

//常规文件权限定义
const int DEF_FMODE = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

/*将data内容写入文件: 
*Note:
*先在fpath相同的目录下写临时文件，再重命名过去
*文件不存在时会创建，存在时会替换整个内容
*并且会跟踪软链接
*
*@return 返回系统定义的错误码(EBADF,....),成功时返回0
*
*/
int OverWriteFile(const char* fpath,
        const std::string& data,
        mode_t mode);

/*
 *一次读取整个文件内容
 *
 * @ret: 成功返回本次读取的数据大小（单位Byte),
 *  文件为空时返回0,失败返回-1
 *
 * Note:
 * 1.不适用于太大的文件!!!
 * 2.会跟踪软链接
 */
int ReadAll(const char* fpath,
            std::string& data);

bool listFilesInDirectory(const std::string& path, std::vector<std::string>& files);

}

#endif  /* UTILS_FILE_UTILS_H_ */
 
 
