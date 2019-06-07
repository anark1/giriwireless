/** @copyright AstroSoft Ltd */

#include "critical_section.hpp"
#include "semaphore.hpp"

namespace macs
{

Semaphore::Semaphore(size_t start_count, size_t max_count) :
		m_count(start_count <= max_count ? start_count : max_count),
		m_max_count(max_count)
{
}

Semaphore::~Semaphore()
{
}

Result Semaphore::Wait(uint32_t timeout_ms)
{
	if (!Sch().IsInitialized() || !Sch().IsStarted())
		return ResultErrorInvalidState;

	if (timeout_ms == 0) {
		if (!System::IsSysCallAllowed())
			return ResultErrorSysCallNotAllowed;
	} else {
		if (System::IsInInterrupt())
			return ResultErrorInterruptNotSupported;
	}

	Result res = System::IsInPrivOrIrq() ? Wait_Priv(this, timeout_ms) : SvcExecPrivileged(this, reinterpret_cast<void*>(timeout_ms), NULL, EPM_Semaphore_Wait_Priv);
	if (res != ResultOk)
		return res;

	return Task::GetCurrent()->m_unblock_reason == Task::UnblockReasonTimeout ? ResultTimeout : ResultOk;
}

Result Semaphore::Wait_Priv(Semaphore * pS, uint32_t timeout_ms)
{
	CriticalSection _cs_;

	Task * currentTask = Task::GetCurrent();
	if (pS->TryDecrement()) {
		currentTask->m_unblock_reason = Task::UnblockReasonNone;  
		return ResultOk;
	}

	if (timeout_ms == 0)
		return ResultTimeout;

	return pS->BlockCurTask(timeout_ms);
}

Result Semaphore::Signal()
{
	if (!Sch().IsInitialized() || !Sch().IsStarted())
		return ResultErrorInvalidState;

	if (!System::IsSysCallAllowed())
		return ResultErrorSysCallNotAllowed;

	return System::IsInPrivOrIrq() ? Signal_Priv(this) : SvcExecPrivileged(this, NULL, NULL, EPM_Semaphore_Signal_Priv);
}

Result Semaphore::Signal_Priv(Semaphore * pS)
{
	CriticalSection _cs_;

	if (pS->m_count == pS->m_max_count)
		return ResultErrorInvalidState;

	if (pS->IsHolding())
		return pS->UnblockTask();

	++pS->m_count;

	return ResultOk;
}

}  
