#include "hydra_utils.h"
#include <dlfcn.h>
#include <stdio.h>
#include "log/log.h"

typedef int (*HydraInsert)(const char *strFilePath, const char *strType);

namespace hydra_utils {

static void *g_handle = NULL;

int Init(const char *strLibPath)
{
    if (!g_handle) {
        g_handle = dlopen(strLibPath, RTLD_LAZY);
        if (g_handle == NULL) {
            LOG_ERROR("dlopen error");
            printf("dlopen error\n");
            return -1;
        }
    }
    return 0;
}

int Insert(const char *strLibPath, const char *strSym, const char *strFilePath, const char *strType)
{
    int ret = -1;
    HydraInsert funcHydraInsert = NULL;
    if (!strLibPath || !strSym || !strFilePath || !strType) {
        LOG_ERROR("param error");
        printf("param error\n");
        return ret;
    }
    if (Init(strLibPath)) {
        return ret;
    }
    funcHydraInsert = (HydraInsert)dlsym(g_handle, strSym);
    if (funcHydraInsert == NULL) {
        LOG_ERROR("dlsym error");
        printf("dlsym error\n");
        return ret;
    }
    ret = funcHydraInsert(strFilePath, strType);
    LOG_INFO("hydra insert ret=[%d]", ret);
    printf("hydra insert ret=[%d]\n", ret);
    return ret;
}

int Insert(const char *strSym, const char *strFilePath, const char *strType)
{
    int ret = -1;
    HydraInsert funcHydraInsert = NULL;
    if (!strSym || !strFilePath || !strType) {
        LOG_ERROR("param error");
        printf("param error\n");
        return ret;
    }
    if (!g_handle) {
        LOG_ERROR("handle is null");
        printf("handle is null\n");
        return ret;
    }
    funcHydraInsert = (HydraInsert)dlsym(g_handle, strSym);
    if (funcHydraInsert == NULL) {
        LOG_ERROR("dlsym error");
        printf("dlsym error\n");
        return ret;
    }
    ret = funcHydraInsert(strFilePath, strType);
    LOG_INFO("hydra insert ret=[%d]", ret);
    printf("hydra insert ret=[%d]\n", ret);
    return ret;
}

void Uninit()
{
    if (g_handle) {
        dlclose(g_handle);
        g_handle = NULL;
    }
}

} 
 
