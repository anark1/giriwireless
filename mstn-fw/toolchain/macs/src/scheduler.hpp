/** @copyright AstroSoft Ltd */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include "common.hpp"
#include "task.hpp"
#include "critical_section.hpp"

namespace macs
{

extern "C"
{
extern bool SchedulerSysTickHandler();
extern StackPtr SchedulerSwitchContext(StackPtr new_sp);
extern void MacsIrqHandler();
}

class TaskRoom
{
public:
	Task * m_task_list;
public:
	TaskRoom()
	{
		m_task_list = nullptr;
	}
	Task * FirstTask()
	{
		return m_task_list;
	}
	const Task * FirstTask() const
	{
		return m_task_list;
	}
	ulong Qty() const
	{
		return TaskRoomList::Qty(m_task_list);
	}
#if MACS_DEBUG	
	inline bool IsInList(Task * task)
	{
		return !!*TaskRoomList::Find(m_task_list, task);
	}
#endif	
	inline void Remove(Task * task)
	{
		TaskRoomList::Del(m_task_list, task);
	}
};

class TaskSleepRoom: public TaskRoom
{
public:
	inline void Insert(Task * task);
	inline Task * Fetch()
	{
		return (m_task_list && !m_task_list->m_dream_ticks) ? TaskSleepList::Fetch(m_task_list) : nullptr;
	}

	void Tick();
};

class TaskWorkRoom: public TaskRoom
{
public:
	inline void Insert(Task * task);
	inline Task * Fetch()
	{
		return TaskWorkList::Fetch(m_task_list);
	}
};

SLIST_DECLARE(TaskIrqList, TaskIrq, m_next_irq_task);
class TaskIrqRoom
{
private:
	TaskIrq * m_irq_task_list;
	bool m_event;  
public:
	TaskIrqRoom()
	{
		m_event = false;
		m_irq_task_list = nullptr;
	}

	inline void Add(TaskIrq * task)
	{
		TaskIrqList::Add(m_irq_task_list, task);
	}
	inline void Del(TaskIrq * task)
	{
		TaskIrqList::Del(m_irq_task_list, task);
	}

	void ProceedIrq(int irq_num);

	void ActivateTasks();
	bool NeedIrqActivate()
	{
		return m_event;
	}
};

class Scheduler
{
public:
	 
	static inline Scheduler & GetInstance()
	{
		return m_instance;
	}

	Result Initialize();

	inline bool IsInitialized()
	{
		return m_initialized;
	}
	 
	Result Start(bool use_preemption = true);
	 
	inline bool IsStarted()
	{
		return m_started;
	}
	 
	uint32_t GetTickCount() const
	{
		return m_tick_count;
	}
	 
	Result RemoveTask(Task * task)
	{
		return DeleteTask(task, false);
	}
	 
	Result DeleteTask(Task * task)
	{
		return DeleteTask(task, true);
	}

	Result BlockCurrentTask(uint32_t timeout_ms = INFINITE_TIMEOUT, Task::UnblockFunctor * unblock_functor = nullptr);
	Result UnblockTask(Task * task);
	Result SetTaskPriority(Task * task, Task::Priority priority);
	 
	inline Task * GetCurrentTask() const
	{
		return m_cur_task;
	}

	inline void Yield()
	{
		if (!m_started)
			return;

		System::IsInPrivOrIrq() ? Yield_Priv(this) : (void)SvcExecPrivileged(this, NULL, NULL, EPM_Yield_Priv);
	}
	 
	void ProceedIrq(int irq_num)
	{
		m_irq_tasks.ProceedIrq(irq_num);
	}
	 
	Result Pause(bool set_on);
	 
	uint GetTasksQty();

private:
	Scheduler();
	~Scheduler();

	CLS_COPY(Scheduler)

	bool UnblockTaskInternal(Task * task, Task::UnblockReason reason);
	void ForceContextSwitch();
	void SelectNextTask();
public:
	StackPtr SwitchContext(StackPtr new_sp);
	inline void TryContextSwitch()
	{  
		if (!m_pause_flg && m_pause_cnt == 0)
			System::SwitchContext();
		else
			m_pending_swc = true;
	}
private:
	bool SysTickHandler();
	bool IsContextSwitchRequired();
	bool IsPriorityValid(Task::Priority priority);
	void TuneProfiler();

	 
	Result AddTask(Task * task, Task::Priority priority = Task::PriorityNormal, Task::Mode mode = Task::ModeUnprivileged, size_t stack_size = Task::MIN_STACK_SIZE);
	Result AddTask(TaskIrq * task, int irq_num, Task::Priority priority, Task::Mode mode, size_t stack_size);
	Result DeleteTask(Task * task, bool del_mem);

	friend StackPtr SchedulerSwitchContext(StackPtr new_sp);
	friend bool SchedulerSysTickHandler();
	friend void Yield_Priv(Scheduler * pS);
	friend Result AddTask_Priv(Scheduler * pS, Task * task);
	friend Result AddTaskIrq_Priv(Scheduler * pS, TaskIrq * task);
	friend Result BlockCurrentTask_Priv(Scheduler * pS, uint32_t timeout_ms, Task::UnblockFunctor *);
	friend Result DeleteTask_Priv(Scheduler * pS, Task * task, bool del_mem);
	friend Result UnblockTask_Priv(Scheduler * pS, Task * task);
#if MACS_MUTEX_PRIORITY_INVERSION
	friend Result IntSetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority, bool internal_usage);
#else
	friend Result SetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority);
#endif
	friend void _SetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority);
	friend class Task;
	friend class TaskIrq;
	friend class TaskIrqRoom;
#if MACS_DEBUG
	friend class TaskSleepRoom;
	friend class TaskWorkRoom;
#endif		
	friend class PauseSection;
	friend void MacsIrqHandler();

	static Scheduler m_instance;

	TaskSleepRoom m_sleep_tasks;
	TaskWorkRoom m_work_tasks;
	TaskIrqRoom m_irq_tasks;

	Task * m_cur_task;  
	volatile uint32_t m_tick_count;

	bool m_initialized;
	bool m_started;
	bool m_pause_flg;
	uint m_pause_cnt;
	bool m_pending_swc;
	bool m_use_preemption;
};
 

inline Scheduler & Sch()
{
	return Scheduler::GetInstance();
}

class PauseSection
{
public:
	PauseSection()
	{
		Sch().Pause(true);
	}
	~PauseSection()
	{
		Sch().Pause(false);
	}
};

extern Result AddTask_Priv(Scheduler * pS, Task * task);
extern Result AddTaskIrq_Priv(Scheduler * pS, TaskIrq * task);
extern void Yield_Priv(Scheduler * pS);
extern Result DeleteTask_Priv(Scheduler * pS, Task * task, bool del_mem);
extern Result UnblockTask_Priv(Scheduler * pS, Task * task);
#if MACS_MUTEX_PRIORITY_INVERSION
extern Result InfSetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority, bool internal_usage);
#endif
extern Result SetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority);
extern void _SetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority);
extern Result BlockCurrentTask_Priv(Scheduler * pS, uint32_t timeout_ms, Task::UnblockFunctor *);
extern uint32_t Read_Cpu_Tick_Priv();

}  

extern Result Spi_Initialize_Priv(void * p);
extern void Spi_PowerControl(void * spi, uint32_t state);
