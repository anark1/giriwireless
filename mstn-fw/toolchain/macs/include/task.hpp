/** @copyright AstroSoft Ltd */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include "system.hpp"
#include "common.hpp"
#include "list.hpp"

namespace macs
{

class Scheduler;
class SyncObject;
class SyncOwnedObject;
class Event;
class Mutex;
class Semaphore;
 
class Task
{
public:
	 
	enum Priority
	{
		PriorityIdle = 0,  
		PriorityLow = 10,  
		PriorityBelowNormal = 20,  
		PriorityNormal = 30,  
		PriorityAboveNormal = 40,  
		PriorityHigh = 50,  
		PriorityRealtime = 60,  
		PriorityMax = MACS_MAX_TASK_PRIORITY,
		PriorityInvalid = MACS_MAX_TASK_PRIORITY + 1
	};

	enum State
	{
		StateReady,
		StateRunning,
		StateBlocked,
		StateInactive
	};

	enum Mode
	{
		ModePrivileged,
		ModeUnprivileged
	};

	enum UnblockReason
	{
		UnblockReasonNone,
		UnblockReasonRequest,
		UnblockReasonTimeout,
		UnblockReasonIrq
	};

	 
	class UnblockFunctor
	{
	public:
		virtual void OnUnblockTask(Task * task, Task::UnblockReason reason) = 0;
		virtual void OnDeleteTask(Task * task)
		{
		}
	};
public:
	 
	static const size_t MIN_STACK_SIZE = TaskStack::MIN_SIZE;
	static const size_t SMALL_STACK_SIZE = (TaskStack::ENOUGH_SIZE + TaskStack::MIN_SIZE) / 2;
	static const size_t ENOUGH_STACK_SIZE = TaskStack::ENOUGH_SIZE;
	static const size_t MAX_STACK_SIZE = TaskStack::MAX_SIZE;

public:
	virtual ~Task();
	 
	const char * GetName() const
	{
#if MACS_TASK_NAME_LENGTH > 0	 
		return m_name_arr;
#elif MACS_TASK_NAME_LENGTH == -1	  	
		return m_name_ptr;
#else		
		return nullptr;
#endif		
	}

	State GetState() const
	{
		return m_state;
	}
	 
	static Result Add(Task * task, Task::Priority priority, Task::Mode mode, size_t stack_size = Task::ENOUGH_STACK_SIZE);

	static inline Result Add(Task * task, Task::Mode mode, Task::Priority priority, size_t stack_size = Task::ENOUGH_STACK_SIZE)
	{
		return Add(task, priority, mode, stack_size);
	}
	 
	static inline Result Add(Task * task, size_t stack_size = Task::ENOUGH_STACK_SIZE)
	{
		return Add(task, Task::PriorityNormal, Task::ModeUnprivileged, stack_size);
	}
	 
	static inline Result Add(Task * task, Task::Priority priority, size_t stack_size = Task::ENOUGH_STACK_SIZE)
	{
		return Add(task, priority, Task::ModeUnprivileged, stack_size);
	}
	 
	static inline Result Add(Task * task, Task::Mode mode, size_t stack_size = Task::ENOUGH_STACK_SIZE)
	{
		return Add(task, Task::PriorityNormal, mode, stack_size);
	}
	 
	Result Remove();
	Result Delete();
	static Result Delay(uint32_t timeout_ms);
	static void CpuDelay(uint32_t timeout_ms);
	 
	inline Priority GetPriority() const
	{
		return m_priority;
	}

	Result SetPriority(Priority value);
	static Task * GetCurrent();
	static void Yield();
	 
	inline size_t GetStackLen() const
	{
		return m_stack.GetLen();
	}
	 
	inline size_t GetStackUsage() const
	{
		return m_stack.GetUsage();
	}
	 
	void InstrumentStack()
	{
		m_stack.Instrument();
	}

protected:
	 
	Task(const char * name = nullptr)
	{
		Init(name, 0, nullptr);
	}
	 
	Task(size_t stack_len, uint32_t * stack_mem, const char * name = nullptr)
	{
		Init(name, stack_len, stack_mem);
	}

private:
	CLS_COPY(Task)

	void Init(const char * name, size_t stack_len, uint32_t * stack_mem);

	 
	void InitializeStack(size_t stack_len, void (*onTaskExit)(void));

	bool IsRunnable() const
	{
		return m_state == Task::StateRunning || m_state == Task::StateReady;
	}
	 
	virtual void Execute() = 0;

public:
	static void Execute_(Task * task)
	{
		task->Execute();
		task->Remove();
	}

private:
	inline uint32_t GetExecuteAddress()
	{
		return (uint32_t) & Task::Execute_;
	}

	void SetBlockSync(SyncObject *);  
	void AddOwnedSync(SyncOwnedObject *);  
	void DropBlockSync(SyncObject *);  
	void RemoveOwnedSync(SyncOwnedObject *);  
	void DetachFromSync();  

	friend class Scheduler;
	friend class SyncObject;
	friend class Mutex;
	friend class Semaphore;
	friend class Event;
	friend class TaskRoom;
	friend class TaskSleepRoom;
	friend class TaskIrqRoom;
	friend Result DeleteTask_Priv(Scheduler * pS, Task * task, bool del_mem);
	friend Result BlockCurrentTask_Priv(Scheduler * pS, uint32_t timeout_ms, Task::UnblockFunctor *);
	friend Result UnblockTask_Priv(Scheduler * pS, Task * task);
#if MACS_MUTEX_PRIORITY_INVERSION
	friend Result IntSetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority, bool internal_usage);
#else
	friend Result SetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority);
#endif
	friend void _SetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority);
	friend bool PriorPreceeding(Task * a, Task * b);
	friend bool WakeupPreceeding(Task * a, Task * b);

	 
#if MACS_TASK_NAME_LENGTH > 0	 
	char m_name_arr[MACS_TASK_NAME_LENGTH + 1];
#elif MACS_TASK_NAME_LENGTH == -1	  	
	const char * m_name_ptr;
#endif	

public:
	TaskStack m_stack;

private:
	Priority m_priority;
	State m_state;
	Mode m_mode;

	uint32_t m_dream_ticks;  
public:
	Task * m_next_sched_task;  
	Task * m_next_sync_task;  
private:
	UnblockFunctor * m_unblock_func;  
	SyncOwnedObject * m_owned_obj_list;  

	 
	UnblockReason m_unblock_reason;
};

inline bool PriorPreceeding(Task * a, Task * b)
{
	return a->m_priority > b->m_priority;
}
inline bool WakeupPreceeding(Task * a, Task * b)
{
	return a->m_dream_ticks <= b->m_dream_ticks;
}

SLISTORD_DECLARE(TaskSyncList, Task, m_next_sync_task, PriorPreceeding);
SLIST_DECLARE(TaskRoomList, Task, m_next_sched_task);
SLISTORD_DECLARE(TaskWorkList, Task, m_next_sched_task, PriorPreceeding);
SLISTORD_DECLARE(TaskSleepList, Task, m_next_sched_task, WakeupPreceeding);

inline Task::Priority operator +(const Task::Priority prior, const int chg)
{
	Task::Priority res = (Task::Priority)((int)prior + chg);
	return res <= Task::PriorityMax ? res : Task::PriorityMax;
}
inline Task::Priority operator -(const Task::Priority prior, const int chg)
{
	return prior + (-chg);
}

inline Task::Priority RandPriority(const Task::Priority max_prior = Task::PriorityMax, const Task::Priority min_prior = Task::PriorityIdle + 1)
{
	_ASSERT(max_prior > min_prior);
	return min_prior + RandN(max_prior - min_prior - 1);
}

extern void PrintPriority(String & str, Task::Priority prior, bool brief = true);

class TaskNaked: public Task
{
private:
	void (*m_exec_func)(Task *);

public:
	 
	TaskNaked(void (*exec_func)(Task *), const char * name = nullptr) :
			Task(name)
	{
		_ASSERT(exec_func);
		m_exec_func = exec_func;
	}

	virtual void Execute()
	{
		(*m_exec_func)(this);
	}
};

class TaskIrq: public Task
{
private:
	bool m_irq_up;  
public:
	TaskIrq * m_next_irq_task;  

protected:
	int m_irq_num;  

	TaskIrq(const char * name = nullptr);

public:
	 
	static Result Add(TaskIrq * task, int irq_num, Task::Priority priority, Task::Mode mode, size_t stack_size = Task::ENOUGH_STACK_SIZE);

	static inline Result Add(TaskIrq * task, int irq_num, Task::Mode mode, Task::Priority priority, size_t stack_size = Task::ENOUGH_STACK_SIZE)
	{
		return Add(task, irq_num, priority, mode, stack_size);
	}
	 
	static inline Result Add(TaskIrq * task, int irq_num, size_t stack_size = Task::ENOUGH_STACK_SIZE)
	{
		return Add(task, irq_num, Task::PriorityNormal, Task::ModeUnprivileged, stack_size);
	}
	 
	static inline Result Add(TaskIrq * task, int irq_num, Task::Priority priority, size_t stack_size = Task::ENOUGH_STACK_SIZE)
	{
		return Add(task, irq_num, priority, Task::ModeUnprivileged, stack_size);
	}
	 
	static inline Result Add(TaskIrq * task, int irq_num, Task::Mode mode, size_t stack_size = Task::ENOUGH_STACK_SIZE)
	{
		return Add(task, irq_num, Task::PriorityNormal, mode, stack_size);
	}
	 
	static inline bool SetUp(int irq_num, bool vector = true, bool enable = true)
	{
		return System::SetUpIrqHandling(irq_num, vector, enable);
	}
	 
	virtual void IrqHandler() = 0;
private:
	 
	virtual void Execute();

	friend class TaskRoom;
	friend class TaskIrqRoom;
	friend class Scheduler;
	friend Result AddTaskIrq_Priv(Scheduler * pS, TaskIrq * task);
};

class SyncObject: public Task::UnblockFunctor
{
public:
	Task * m_blocked_task_list;
public:
	SyncObject()
	{
		m_blocked_task_list = nullptr;
	}

	inline bool IsHolding() const
	{
		return !!m_blocked_task_list;
	}

	virtual Result BlockCurTask(uint32_t timeout_ms);
	virtual Result UnblockTask();
	virtual void OnUnblockTask(Task *, Task::UnblockReason);
	virtual void OnDeleteTask(Task *);

protected:
	void DropLinks();
};

class SyncOwnedObject: public SyncObject
{
public:
	Task * m_owner;
	Task::Priority m_owner_original_priority;
	SyncOwnedObject * m_next_owned_obj;
public:
	SyncOwnedObject()
	{
		m_owner = nullptr;
		m_next_owned_obj = nullptr;
	}
};
SLIST_DECLARE(OwnedSyncObjList, SyncOwnedObject, m_next_owned_obj);

}  
