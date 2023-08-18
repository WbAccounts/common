#ifndef HYDRA_UTILS_H
#define HYDRA_UTILS_H

namespace hydra_utils {

int Init(const char *strLibPath);
int Insert(const char *strLibPath, const char *strSym, const char *strFilePath, const char *strType);
int Insert(const char *strSym, const char *strFilePath, const char *strType);
void Uninit();

}

#endif // HYDRA_UTILS_H 
 
