#ifndef IASUnknown_h
#define IASUnknown_h

#include "ASErrCode.h"

class IASUnknown
{
public:
    
    virtual ASCode	QueryInterface(const char* pszClsid,void** ppInterface) = 0;
    virtual long	AddRef() = 0;
    virtual long	Release() = 0;
};

#if (defined _WINDOWS) || (defined WIN32) || (defined WIN64)
#define ASUNKNOWN_EASY_IMPLEMENT(theClass)\
public:\
	virtual ASCode	QueryInterface(const char* pszClsid, void** ppInterface) { return ASErr_NOIMPL; }\
	virtual long	AddRef() { return InterlockedIncrement(&m_lRefCount_##theClass); }\
	virtual long	Release() { long l = InterlockedDecrement(&m_lRefCount_##theClass); if (l == 0) delete this; return l; }\
private:\
	long m_lRefCount_##theClass;
#else
#define ASUNKNOWN_EASY_IMPLEMENT(theClass)\
public:\
	virtual ASCode	QueryInterface(const char* pszClsid, void** ppInterface) { return ASErr_NOIMPL; }\
	virtual long	AddRef() { return __sync_add_and_fetch(&m_lRefCount_##theClass, 1); }\
	virtual long	Release() { long l = __sync_sub_and_fetch(&m_lRefCount_##theClass, 1); if (0 == l) delete this; return l; }\
private:\
	volatile long m_lRefCount_##theClass;
#endif

#endif /* IASUnknown_h */
 
 
