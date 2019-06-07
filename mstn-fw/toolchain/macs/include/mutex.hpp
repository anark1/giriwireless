#pragma once
#include "scheduler.hpp"

namespace macs
{

class Mutex: public SyncOwnedObject
{
private:
	uint32_t m_lock_cnt;
	bool m_recursive;
public:
	Mutex(bool recursive = false) :
			m_lock_cnt(0),
			m_recursive(recursive)
	{
	}

	~Mutex();

	bool IsRecursive() const
	{
		return m_recursive;
	}

	virtual Result Lock(uint32_t timeout_ms = INFINITE_TIMEOUT);
	virtual Result Unlock();

	bool IsLocked() const
	{
		return !!m_owner;
	}

	static Result Lock_Priv(Mutex * pM, uint32_t timeout_ms);
	static Result Unlock_Priv(Mutex * pM);

private:
	CLS_COPY(Mutex)

	virtual Result BlockCurTask(uint32_t timeout_ms);
	virtual Result UnblockTask();
	virtual void OnUnblockTask(Task *, Task::UnblockReason);
	virtual void OnDeleteTask(Task *);
#if MACS_MUTEX_PRIORITY_INVERSION
	inline Task::Priority RemoveFromOwner()
#else
	inline void RemoveFromOwner()
#endif
	{
		m_owner->RemoveOwnedSync(this);
#if MACS_MUTEX_PRIORITY_INVERSION
		Task::Priority inh_prior = m_owner_original_priority;
		SyncOwnedObject * pobj = m_owner->m_owned_obj_list;
		while (pobj) {
			if (pobj->m_blocked_task_list) {
				Task::Priority prior = pobj->m_blocked_task_list->GetPriority();
				if (prior > inh_prior)
					inh_prior = prior;
			}
			pobj = OwnedSyncObjList::Next(pobj);
		}
		return inh_prior;
#endif
	}

	Result UnlockInternal();
	void UpdateOwnerPriority();
};

class MutexGuard
{
public:
	explicit MutexGuard(Mutex & mutex, bool only_unlock = false) :
			m_mutex(mutex)
	{
		if (!only_unlock)
			m_mutex.Lock();
	}

	~MutexGuard()
	{
		m_mutex.Unlock();
	}

private:
	CLS_COPY(MutexGuard)

private:
	Mutex & m_mutex;
};

}
