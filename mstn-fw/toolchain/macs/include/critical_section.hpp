/** @copyright AstroSoft Ltd */
#pragma once

#include <stdint.h>
#include "system.hpp"
#include "application.hpp"

namespace macs
{
 
class CriticalSection
{
public:
	inline CriticalSection()
	{
		if (!System::IsInPrivOrIrq())
			App().OnAlarm(AR_NOT_IN_PRIVILEGED);

		m_prev_interrupt_mask = System::DisableIrq();
	}
	inline ~CriticalSection()
	{
		System::EnableIrq(m_prev_interrupt_mask);
	}

private:
	CLS_COPY(CriticalSection)
private:
	uint32_t m_prev_interrupt_mask;
};

}  
