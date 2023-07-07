/*
 *notifier_chain.hpp: 2022-07-20 created by qudreams
 *通知链工具类，类似内核的notifier chain
 */

#pragma once

#include <sys/types.h>
#include "qh_thread/locker.hpp"

namespace notifier_chain {

struct CNotifierBlock {
	u_int (*notifier_call)(CNotifierBlock*, u_long, void *);
	CNotifierBlock *next;
	int priority; //优先级，降序方式，值越大调用越早
	void* data; //附加数据，由调用者使用
};

enum {
	NOTIFY_DONE = 0x0000,		/* Don't care */
	NOTIFY_OK	= 0x0001,		/* Suits me */
	NOTIFY_STOP_MASK =	0x8000, /* Don't call further */
	NOTIFY_BAD	=	(NOTIFY_STOP_MASK|0x0002), /* Bad/Veto action */
	//Clean way to return from the notifier and stop further calls.
	NOTIFY_STOP	=	(NOTIFY_OK|NOTIFY_STOP_MASK) 
};

//阻塞形式的通知链,这个实现要尽可能高效，
//因为我们代码中很多需要通知链的地方都是跨模块且加锁很重的地方！！！
class CBlockNotifierChain
{
private:
    CBlockNotifierChain(const CBlockNotifierChain&);
    void operator=(const CBlockNotifierChain&);
public:
    CBlockNotifierChain();
    ~CBlockNotifierChain();

    //这里直接使用指针是为了性能，跟内核类型，
    //我们能不做任何复制操作就不做任何复制操作
    //外围要保证注册后notifier是有效的!!!
    bool Register(CNotifierBlock* notifier);
    bool Unregister(CNotifierBlock* notifier);

    u_int Call(u_long ev, void *val);

private:
    QH_THREAD::CRwlock m_rwLock;
    CNotifierBlock* m_notifierHead;
};

//无锁阻塞形式的通知链,这个实现要尽可能高效，
//因为我们代码中很多需要通知链的地方都是跨模块且加锁很重的地方！！！
class CRawBlockNotifierChain
{
private:
    CRawBlockNotifierChain(const CRawBlockNotifierChain&);
    void operator=(const CRawBlockNotifierChain&);
public:
    CRawBlockNotifierChain();
    ~CRawBlockNotifierChain();

    //这里直接使用指针是为了性能，跟内核类型，
    //我们能不做任何复制操作就不做任何复制操作
    //外围要保证注册后notifier是有效的!!!
    bool Register(CNotifierBlock* notifier);
    bool Unregister(CNotifierBlock* notifier);

    u_int Call(u_long ev, void *val);

private:
    CNotifierBlock* m_notifierHead;
};


}; //end namespace notifier_chain 

using CNotifierBlock = notifier_chain::CNotifierBlock;
 
