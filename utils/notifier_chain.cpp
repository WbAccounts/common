#include "notifier_chain.h"

namespace notifier_chain {
/*
 *	Notifier chain core routines.  The exported routines below
 *	are layered on top of these, with appropriate locking added.
 */

static int notifier_chain_register(CNotifierBlock **nl,
		                        CNotifierBlock *n)
{
	while ((*nl) != NULL) {
		if (n->priority > (*nl)->priority)
			break;
		nl = &((*nl)->next);
	}
	n->next = *nl;
	*nl = n;

	return 0;
}

static int notifier_chain_unregister(CNotifierBlock **nl,
		                        CNotifierBlock *n)
{
	while ((*nl) != NULL) {
		if ((*nl) == n) {
			*nl = n->next;
			return 0;
		}
		nl = &((*nl)->next);
	}
	return -1;
}

static u_int notifier_call_chain(CNotifierBlock **nl,
		                        u_long ev, void *val)
{
	u_int ret = NOTIFY_DONE;
	CNotifierBlock *nb, *next_nb;

	nb = *nl;
	while (nb) {
		next_nb = nb->next;
		ret = nb->notifier_call(nb, ev, val);
		if ((ret & NOTIFY_STOP_MASK) == NOTIFY_STOP_MASK)
			break;
		nb = next_nb;
	}
	return ret;
}

CBlockNotifierChain::CBlockNotifierChain() :
        m_notifierHead(NULL)
{}

CBlockNotifierChain::~CBlockNotifierChain()
{}

bool CBlockNotifierChain::Register(CNotifierBlock* notifier)
{
    QH_THREAD::CWriteAutoLocker locker(&m_rwLock);
    return (notifier_chain_register(&m_notifierHead,notifier) == 0);
}

bool CBlockNotifierChain::Unregister(CNotifierBlock* notifier)
{
    QH_THREAD::CWriteAutoLocker locker(&m_rwLock);
    return (notifier_chain_unregister(&m_notifierHead,notifier) == 0);
}

u_int CBlockNotifierChain::Call(u_long ev, void *val)
{
    QH_THREAD::CReadAutoLocker locker(&m_rwLock);
    return notifier_call_chain(&m_notifierHead,ev,val);
}

//无锁通知链
CRawBlockNotifierChain::CRawBlockNotifierChain() :
        m_notifierHead(NULL)
{}

CRawBlockNotifierChain::~CRawBlockNotifierChain()
{}

bool CRawBlockNotifierChain::Register(CNotifierBlock* notifier)
{
    return (notifier_chain_register(&m_notifierHead,notifier) == 0);
}

bool CRawBlockNotifierChain::Unregister(CNotifierBlock* notifier)
{
    return (notifier_chain_unregister(&m_notifierHead,notifier) == 0);
}

u_int CRawBlockNotifierChain::Call(u_long ev, void *val)
{
    return notifier_call_chain(&m_notifierHead,ev,val);
}

}