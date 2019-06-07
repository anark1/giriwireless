/** @copyright AstroSoft Ltd */

#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include "memory_manager.hpp"
#include "scheduler.hpp"
#include "system.hpp"
#include "common.hpp"
#include "critical_section.hpp"
#include "event.hpp"
#include "stack_frame.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"
#include "list.hpp"
#include "profiler.hpp"

namespace macs
{

extern "C" void * svcMethods[] = {
	reinterpret_cast<void *>(EPM_Count),
	reinterpret_cast<void *>(&Read_Cpu_Tick_Priv),
	reinterpret_cast<void *>(&BlockCurrentTask_Priv),
	reinterpret_cast<void *>(&AddTask_Priv),
	reinterpret_cast<void *>(&AddTaskIrq_Priv),
	reinterpret_cast<void *>(&Yield_Priv),
	reinterpret_cast<void *>(&DeleteTask_Priv),
	reinterpret_cast<void *>(&UnblockTask_Priv),
	reinterpret_cast<void *>(&SetTaskPriority_Priv),
	reinterpret_cast<void *>(&Event::Raise_Priv),
	reinterpret_cast<void *>(&Event::Wait_Priv),
	reinterpret_cast<void *>(&Mutex::Lock_Priv),
	reinterpret_cast<void *>(&Mutex::Unlock_Priv),
	reinterpret_cast<void *>(&Semaphore::Wait_Priv),
	reinterpret_cast<void *>(&Semaphore::Signal_Priv)
#if MACS_SHARED_MEM_SPI
	,
	reinterpret_cast<void *>(&Spi_Initialize_Priv),
	reinterpret_cast<void *>(&Spi_PowerControl)
#endif			
};

extern "C" void SvcInitScheduler();

#if MACS_DEBUG
unsigned long IdleTaskCnt;  
#endif	
class IdleTask: public Task
{
public:
	IdleTask() :
			Task("IDLE")
	{
	}

private:
	virtual void Execute()
	{
		for (;;) {
#if MACS_SLEEP_ON_IDLE
			System::EnterSleepMode();
#endif			
#if MACS_DEBUG
			++IdleTaskCnt;
#endif			
		}
	}
};

Scheduler::Scheduler() :
		m_cur_task(nullptr),
		m_tick_count(0),
		m_initialized(false),
		m_started(false),
		m_pause_flg(false),
		m_pause_cnt(0),
		m_pending_swc(false),
		m_use_preemption(true)
{
}
Scheduler Scheduler::m_instance;

Scheduler::~Scheduler()
{
}

Result Scheduler::Initialize()
{
	if (System::IsInInterrupt())
		return ResultErrorInterruptNotSupported;

	if (m_initialized)
		return ResultErrorInvalidState;

#if MACS_PROFILING_ENABLED
	TuneProfiler();
#endif

#if MACS_USE_MPU
	MPU_Init();
#endif		

	m_tick_count = 0;

	if (!System::InitScheduler())
		return ResultErrorInvalidState;
	 
	AddTask(new IdleTask(), Task::PriorityIdle, Task::ModePrivileged);

	m_initialized = true;
	return ResultOk;
}

Result Scheduler::Start(bool use_preemption)
{
	if (System::IsInInterrupt())
		return ResultErrorInterruptNotSupported;

	if (!m_initialized || m_started)
		return ResultErrorInvalidState;

	if (!System::IsInPrivMode() || !System::IsInMspMode())
		return ResultErrorInvalidState;

	m_use_preemption = use_preemption;

	SelectNextTask();

#if MACS_MPU_PROTECT_STACK
	m_cur_task->m_stack.SetMpuMine();
#endif

	m_started = true;

	System::FirstSwitchToTask(m_cur_task->m_stack.m_top, m_cur_task->m_mode == Task::ModePrivileged);
	 
	return ResultOk;
}

Result Scheduler::Pause(bool set_on)
{
	if (!m_started)
		return ResultErrorInvalidState;

	if (!set_on) {
		 
		if (m_pause_cnt == 0) {
			App().OnAlarm(AR_SCHED_NOT_ON_PAUSE);

			return ResultErrorInvalidState;
		}
		if (--m_pause_cnt == 0) {
			if (m_pending_swc) {
				Yield();
			}
		}
	} else {
		m_pause_flg = true;  
		++m_pause_cnt;  
		if (m_pause_cnt == 0)
			App().OnAlarm(AR_COUNTER_OVERFLOW);
		m_pause_flg = false;  
	}

	return ResultOk;
}

void Scheduler::ForceContextSwitch()
{
	System::SwitchContext();
}

void Yield_Priv(Scheduler * pS)
{
	CriticalSection _cs_;

	if (pS->IsContextSwitchRequired())
		pS->TryContextSwitch();
}

static void OnTaskExit()
{
	Scheduler & shed = Sch();
	shed.RemoveTask(shed.GetCurrentTask());
}

Result Scheduler::AddTask(Task * task, Task::Priority priority, Task::Mode mode, size_t stack_size)
{
	if (System::IsInInterrupt() && !System::IsInSysCall())
		return ResultErrorInterruptNotSupported;

	if (!task || !IsPriorityValid(priority))
		return ResultErrorInvalidArgs;

	if (task->m_state != Task::StateInactive)
		return ResultErrorInvalidState;

	task->InitializeStack(stack_size, OnTaskExit);
	task->m_priority = priority;
	task->m_state = Task::StateReady;
#if MACS_PROFILING_ENABLED
	task->m_mode = Task::ModePrivileged;
#else		
	task->m_mode = mode;
#endif		

	return System::IsInPrivOrIrq() ? AddTask_Priv(this, task) : SvcExecPrivileged(this, task, NULL, EPM_AddTask_Priv);
}

Result AddTask_Priv(Scheduler * pS, Task * task)
{
	CriticalSection _cs_;

	pS->m_work_tasks.Insert(task);

	if (pS->m_use_preemption)
		pS->Yield();

	return ResultOk;
}

Result Scheduler::AddTask(TaskIrq * task, int irq_num, Task::Priority priority, Task::Mode mode, size_t stack_size)
{
	if (System::IsInInterrupt() && !System::IsInSysCall())
		return ResultErrorInterruptNotSupported;

	if (!task || !IsPriorityValid(priority))
		return ResultErrorInvalidArgs;

	if (task->m_state != Task::StateInactive)
		return ResultErrorInvalidState;

	task->InitializeStack(stack_size, OnTaskExit);
	_ASSERT(task->m_irq_num == -1);
	task->m_irq_num = irq_num;
	task->m_priority = priority;
	task->m_state = Task::StateBlocked;
#if MACS_PROFILING_ENABLED
	task->m_mode = Task::ModePrivileged;
#else		
	task->m_mode = mode;
#endif		

	return System::IsInPrivOrIrq() ? AddTaskIrq_Priv(this, task) : SvcExecPrivileged(this, task, NULL, EPM_AddTaskIrq_Priv);
}

extern "C" void MacsIrqHandler()
{
	CriticalSection _cs_;
	int inum = System::CurIrqNum();
	Sch().ProceedIrq(inum);
}

Result AddTaskIrq_Priv(Scheduler * pS, TaskIrq * task)
{
	CriticalSection _cs_;
	pS->m_irq_tasks.Add(task);
	return ResultOk;
}

Result DeleteTask_Priv(Scheduler * pS, Task * task, bool del_mem)
{
	CriticalSection _cs_;

	if (task->m_state == Task::StateInactive)
		return ResultErrorInvalidState;

	const bool is_suicide = (task == pS->m_cur_task);

	if (!is_suicide) {
		pS->m_sleep_tasks.Remove(task);
		if (task->IsRunnable())
			pS->m_work_tasks.Remove(task);
	}

	task->DetachFromSync();

	pS->m_irq_tasks.Del((TaskIrq *)task);

#if MACS_MPU_PROTECT_STACK
	if (is_suicide)
		MPU_RemoveMine (MPU_PROC_STACK_MINE);
#endif

	task->m_state = Task::StateInactive;

	if (del_mem)
		delete task;

	if (is_suicide) {
		pS->m_cur_task = nullptr;  
		 
		System::InternalSwitchContext();  
	}

	return ResultOk;
}

Result Scheduler::DeleteTask(Task * task, bool del_mem)
{
	if (System::IsInInterrupt())
		return ResultErrorInterruptNotSupported;

	if (!task)
		return ResultErrorInvalidArgs;

	if (m_started)
		return SvcExecPrivileged(this, task, reinterpret_cast<void*>(del_mem), EPM_DeleteTask_Priv);

	return DeleteTask_Priv(this, task, del_mem);  
}

Result Scheduler::BlockCurrentTask(uint32_t timeout_ms, Task::UnblockFunctor * unblock_functor)
{
	Result res = System::IsInPrivOrIrq() ? BlockCurrentTask_Priv(this, timeout_ms, unblock_functor) : SvcExecPrivileged(this, reinterpret_cast<void*>(timeout_ms), unblock_functor, EPM_BlockCurrentTask_Priv);
	if (res != ResultOk)
		return res;

	return m_cur_task->m_unblock_reason == Task::UnblockReasonTimeout ? ResultTimeout : ResultOk;
}

Result BlockCurrentTask_Priv(Scheduler * pS, uint32_t timeout_ms, Task::UnblockFunctor * unblock_functor)
{
	if (!pS->m_started)
		return ResultErrorInvalidState;

	if (System::IsInInterrupt() && !System::IsInSysCall())
		return ResultErrorInterruptNotSupported;

	CriticalSection _cs_;

	if (!pS->m_cur_task->IsRunnable())
		return ResultErrorInvalidState;

	if (timeout_ms == 0) {
		if (unblock_functor)
			unblock_functor->OnUnblockTask(pS->m_cur_task, Task::UnblockReasonTimeout);

		return ResultTimeout;
	}

	pS->m_cur_task->m_state = Task::StateBlocked;
	pS->m_cur_task->m_unblock_reason = Task::UnblockReasonNone;
	pS->m_cur_task->m_unblock_func = unblock_functor;

	pS->m_cur_task->m_dream_ticks = (timeout_ms != INFINITE_TIMEOUT ? MsToTicks(timeout_ms) : ULONG_MAX);
	pS->m_sleep_tasks.Insert(pS->m_cur_task);

	pS->TryContextSwitch();
	 
	return ResultOk;
}

Result UnblockTask_Priv(Scheduler * pS, Task * task)
{
	CriticalSection _cs_;
	 
	pS->m_sleep_tasks.Remove(task);

	if (!pS->UnblockTaskInternal(task, Task::UnblockReasonRequest))
		return ResultErrorInvalidState;

	if (!pS->m_use_preemption)
		return ResultOk;

	if (pS->m_cur_task->m_priority < task->m_priority)
		pS->TryContextSwitch();

	return ResultOk;
}

Result Scheduler::UnblockTask(Task * task)
{
	if (!m_started)
		return ResultErrorInvalidState;

	if (!System::IsSysCallAllowed())
		return ResultErrorSysCallNotAllowed;

	if (!task)
		return ResultErrorInvalidArgs;

	return System::IsInPrivOrIrq() ? UnblockTask_Priv(this, task) : SvcExecPrivileged(this, task, NULL, EPM_UnblockTask_Priv);
}

bool Scheduler::UnblockTaskInternal(Task * task, Task::UnblockReason reason)
{
	if (task->m_state != Task::StateBlocked)
		return false;

	task->m_unblock_reason = reason;
	task->m_state = Task::StateReady;
	if (task != m_cur_task)
		m_work_tasks.Insert(task);

	if (task->m_unblock_func) {
		task->m_unblock_func->OnUnblockTask(task, reason);
		task->m_unblock_func = nullptr;
	}

	return true;
}

void _SetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority)
{
	task->m_priority = priority;
	if (pS->m_use_preemption)
		pS->Yield();
}

Result SetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority)
#if MACS_MUTEX_PRIORITY_INVERSION
{
	return IntSetTaskPriority_Priv(pS, task, priority, false);
}
Result IntSetTaskPriority_Priv(Scheduler * pS, Task * task, Task::Priority priority, bool internal_usage)
#endif
{
	CriticalSection _cs_;

	if (task->m_state == Task::StateInactive)
		return ResultErrorInvalidState;

	if (task->m_priority == priority)
		return ResultOk;

	task->m_priority = priority;

	if (task->m_state == Task::StateReady) {
		pS->m_work_tasks.Remove(task);  
		pS->m_work_tasks.Insert(task);
	}

#if MACS_MUTEX_PRIORITY_INVERSION
	if (!internal_usage) {
		SyncOwnedObject * pobj = task->m_owned_obj_list;
		while (pobj) {
			pobj->m_owner_original_priority = priority;
			pobj = OwnedSyncObjList::Next(pobj);
		}
	}
#endif

	if (pS->m_use_preemption)
		pS->Yield();

	return ResultOk;
}

Result Scheduler::SetTaskPriority(Task * task, Task::Priority priority)
{
	if (!m_started)
		return ResultErrorInvalidState;

	if (System::IsInInterrupt())
		return ResultErrorInterruptNotSupported;

	if (!task || !IsPriorityValid(priority))
		return ResultErrorInvalidArgs;

	return System::IsInPrivOrIrq() ? SetTaskPriority_Priv(this, task, priority) : SvcExecPrivileged(this, task, reinterpret_cast<void*>(priority), EPM_SetTaskPriority_Priv);
}

bool Scheduler::IsPriorityValid(Task::Priority priority)
{
	return priority <= Task::PriorityMax;
}

void Scheduler::TuneProfiler()
{
#if MACS_PROFILING_ENABLED
	ProfEye::Tune();
#endif	
}

bool Scheduler::SysTickHandler()
{
	CriticalSection _cs_;
	++m_tick_count;

	if (!m_started)
		return false;

	m_sleep_tasks.Tick();
	for (;;) {
		Task * awake_task = m_sleep_tasks.Fetch();
		if (!awake_task)
			break;
		if (!UnblockTaskInternal(awake_task, Task::UnblockReasonTimeout)) {
			;
		}
	}

#if ! MACS_IRQ_FAST_SWITCH
	if (m_irq_tasks.NeedIrqActivate())
		m_irq_tasks.ActivateTasks();
#endif

	if (m_pause_flg || m_pause_cnt != 0) {
		m_pending_swc = true;
		return false;
	}

	if (!m_use_preemption)
		return false;

	return IsContextSwitchRequired();
}

bool Scheduler::IsContextSwitchRequired()
{
	if (m_pending_swc)
		return true;

	if (!m_cur_task || m_cur_task->m_state != Task::StateRunning)
		return true;

	const Task * cand_task = m_work_tasks.FirstTask();
	if (cand_task && m_cur_task->GetPriority() <= cand_task->GetPriority())
		return true;

	return false;
}

 
void Scheduler::SelectNextTask()
{
	if (m_cur_task) {
		if (m_cur_task->m_state == Task::StateRunning)
			m_cur_task->m_state = Task::StateReady;

		if (m_cur_task->m_state == Task::StateReady)
			m_work_tasks.Insert(m_cur_task);
	}

	m_cur_task = m_work_tasks.Fetch();
	m_cur_task->m_state = Task::StateRunning;
}

 
StackPtr Scheduler::SwitchContext(StackPtr new_sp)
{
	CriticalSection _cs_;
	_ASSERT(! m_pause_flg && m_pause_cnt == 0);

	m_pending_swc = false;

	if (m_cur_task) {
		m_cur_task->m_stack.m_top = new_sp;

#if MACS_DEBUG
		bool res = m_cur_task->m_stack.Check();
		if (!res)
			m_cur_task = nullptr;
#endif		
	}

#if MACS_IRQ_FAST_SWITCH
	if (m_irq_tasks.NeedIrqActivate())
		m_irq_tasks.ActivateTasks();
#endif

	SelectNextTask();

#if MACS_MPU_PROTECT_STACK
	m_cur_task->m_stack.SetMpuMine();
#endif

	System::SetPrivMode(m_cur_task->m_mode == Task::ModePrivileged);

	return m_cur_task->m_stack.m_top;
}

extern "C" bool SchedulerSysTickHandler()
{
	return Sch().SysTickHandler();
}

void TaskIrqRoom::ProceedIrq(int irq_num)
{
	for (TaskIrq * ptsk = m_irq_task_list; ptsk != nullptr; ptsk = TaskIrqList::Next(ptsk))
		if (ptsk->m_irq_num == irq_num) {
			if (ptsk->GetState() == Task::StateBlocked && !ptsk->m_unblock_func)
				m_event = true;
			ptsk->m_irq_up = true;
		}
#if MACS_IRQ_FAST_SWITCH
	if (m_event && Sch().m_started) {
		Sch().m_pending_swc = true;
		Sch().Yield();
	}
#endif
}

void TaskIrqRoom::ActivateTasks()
{
	for (TaskIrq * ptsk = m_irq_task_list; ptsk != nullptr; ptsk = TaskIrqList::Next(ptsk))
		if (ptsk->m_irq_up && ptsk->GetState() == Task::StateBlocked && !ptsk->m_unblock_func) {
			Sch().m_sleep_tasks.Remove(ptsk);
			Sch().UnblockTaskInternal(ptsk, Task::UnblockReasonIrq);
			ptsk->m_irq_up = false;
		}
	m_event = false;
}
 
uint32_t Read_Cpu_Tick_Priv()
{
	return System::GetCurCpuTick();
}

uint Scheduler::GetTasksQty()
{
	return m_work_tasks.Qty() + m_sleep_tasks.Qty() + (m_cur_task ? 1 : 0);
}
 
void TaskWorkRoom::Insert(Task * task)
{
	_ASSERT(! Sch().m_sleep_tasks.IsInList(task));

	TaskWorkList::Add(m_task_list, task);
}

void TaskSleepRoom::Insert(Task * task)
{
	_ASSERT(task->m_dream_ticks);_ASSERT(! Sch().m_work_tasks.IsInList(task));

	TaskSleepList::Add(m_task_list, task);
}

void TaskSleepRoom::Tick()
{
	for (Task * ptsk = m_task_list; ptsk != nullptr; ptsk = TaskSleepList::Next(ptsk)) {
		_ASSERT(ptsk->m_dream_ticks);
		if (ptsk->m_dream_ticks != ULONG_MAX)
			--ptsk->m_dream_ticks;
	}
}

}  
