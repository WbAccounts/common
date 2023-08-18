#ifndef __IASUnknownPtr__
#define __IASUnknownPtr__

// �ο�ATL�е�CComPtr����ָ�����ڹ���IASUnknownָ��

#include <assert.h>
#include "common/AsFramework/ASUnknown.h"

template <class T>
class _NoAddRefReleaseOnUnknownPtr : public T
{
private:

	long AddRef() = 0;
	long Release() = 0;
};

inline IASUnknown* UnknownPtrAssign(IASUnknown** pp, IASUnknown* lp)
{
	if (pp == NULL)
		return NULL;

	if (lp != NULL)
		lp->AddRef();
	if (*pp)
		(*pp)->Release();
	*pp = lp;
	return lp;
}

inline IASUnknown* UnknownPtrQIPtrAssign(IASUnknown** pp, IASUnknown* lp, const char* pszClsid)
{
	if (pp == NULL)
		return NULL;

	IASUnknown* pTemp = *pp;
	*pp = NULL;
	if (lp != NULL)
		lp->QueryInterface(pszClsid, (void**)pp);
	if (pTemp)
		pTemp->Release();
	return *pp;
}

template<class T>
class CUnknownPtrBase
{
protected:

	CUnknownPtrBase() throw()
	{
		p = NULL;
	}
	CUnknownPtrBase(int nNull) throw()
	{
		assert(nNull == 0);
		(void)nNull;
		p = NULL;
	}
	CUnknownPtrBase(T* lp) throw()
	{
		p = lp;
		if (p != NULL)
			p->AddRef();
	}

public:

	typedef T _PtrClass;
	~CUnknownPtrBase() throw()
	{
		if (p)
			p->Release();
	}
	operator T*() const throw()
	{
		return p;
	}
	T& operator*() const
	{
		assert(p!=NULL);
		return *p;
	}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&() throw()
	{
		assert(p==NULL);
		return &p;
	}
	_NoAddRefReleaseOnUnknownPtr<T>* operator->() const throw()
	{
		assert(p!=NULL);
		return (_NoAddRefReleaseOnUnknownPtr<T>*)p;
	}
	bool operator!() const throw()
	{
		return (p == NULL);
	}
	bool operator<(T* pT) const throw()
	{
		return p < pT;
	}
	bool operator!=(T* pT) const
	{
		return !operator==(pT);
	}
	bool operator==(T* pT) const throw()
	{
		return p == pT;
	}

	// Release the interface and set to NULL
	void Release() throw()
	{
		T* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}
	// Compare two objects for equivalence
// 	bool IsEqualObject(IASUnknown* pOther) throw()
// 	{
// 		if (p == NULL && pOther == NULL)
// 			return true;	// They are both NULL objects
// 
// 		if (p == NULL || pOther == NULL)
// 			return false;	// One is NULL the other is not
// 
// 		CUnknownPtrT<IASUnknown> punk1;
// 		CUnknownPtrT<IASUnknown> punk2;
// 		p->QueryInterface(__nameof(IASUnknown), (void**)&punk1);
// 		pOther->QueryInterface(__nameof(IASUnknown), (void**)&punk2);
// 		return punk1 == punk2;
// 	}
	// Attach to an existing interface (does not AddRef)
	void Attach(T* p2) throw()
	{
		if (p)
			p->Release();
		p = p2;
	}
	// Detach the interface (does not Release)
	T* Detach() throw()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
	ASCode CopyTo(T** ppT) throw()
	{
		assert(ppT != NULL);
		if (ppT == NULL)
			return ASErr_FAIL;
		*ppT = p;
		if (p)
			p->AddRef();
		return ASErr_OK;
	}
	T* p;
};

template<class T>
class CUnknownPtrT: public CUnknownPtrBase<T>
{
public:
	CUnknownPtrT() throw() {}
	CUnknownPtrT(int nNull) throw(): CUnknownPtrBase<T>(nNull) {}
	CUnknownPtrT(T* lp) throw(): CUnknownPtrBase<T>(lp) {}

	CUnknownPtrT(const CUnknownPtrT<T>& lp) throw(): CUnknownPtrBase<T>(lp.p) {}

	T* operator=(T* lp) throw()
	{
		if(*this!=lp)
		{
			return static_cast<T*>(UnknownPtrAssign((IASUnknown**)&CUnknownPtrBase<T>::p, lp));
		}
		return *this;
	}

// 	template <typename Q>
// 	T* operator=(const CUnknownPtrT<Q>& lp) throw()
// 	{
// 		if( !IsEqualObject(lp) )
// 		{
// 			return static_cast<T*>(UnknownPtrQIPtrAssign((IASUnknown**)&p, lp, __nameof(T)));
// 		}
// 		return *this;
// 	}

	T* operator=(const CUnknownPtrT<T>& lp) throw()
	{
		if(*this!=lp)
		{
			return static_cast<T*>(UnknownPtrAssign((IASUnknown**)&(CUnknownPtrBase<T>::p), lp));
		}
		return *this;
	}
};

#endif
 
 
